/*
 * =====================================================================================
 *
 *       Filename: initview.hpp
 *        Created: 07/18/2017 16:00:20
 *  Last Modified: 08/13/2017 23:08:18
 *
 *    Description: a seperate procedure to load all resource at beginning
 *                 this procedure has its own event loop and two threads
 *
 *                 thread-1: event loop for a small window to view what's loading
 *                 thread-2: loading the resource
 *
 *                 requirement:
 *                 1. SDL
 *                 2. 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <array>
#include <mutex>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <functional>

#include "xmlconf.hpp"
#include "sdldevice.hpp"

class InitView final
{
    private:
        enum LogIVType: int
        {
            LOGIV_INFO    = 0,
            LOGIV_WARNING = 1,
            LOGIV_FATAL   = 2,
        };

        enum IVProcType: int
        {
            IVPROC_LOOP  = 0,
            IVPROC_DONE  = 1,
            IVPROC_ERROR = 2,
        };

    private:
        struct LoadProc
        {
            // this struct may change a lot
            // avoid to use the std::pair or std::tuple
            size_t Weight;

            // what to execute during this event
            // return false means need to stop all rest events
            std::function<bool(size_t)> Event;

            LoadProc(size_t nWeight, const std::function<bool(size_t)> &fnEvent)
                : Weight(nWeight)
                , Event(fnEvent)
            {}
        };

        struct MessageEntry
        {
            // LogType defines different color
            // 0 : normal  : white
            // 1 : warning : yellow
            // 2 : error   : red
            int Type;

            std::string  Message;
            SDL_Texture *Texture;

            MessageEntry(int nLogType = 0, const char *szMessage = "", SDL_Texture *pTexture = nullptr)
                : Type(nLogType)
                , Message(szMessage ? szMessage : "")
                , Texture(pTexture)
            {}
        };

    private:
        static const int m_ButtonX = 345;
        static const int m_ButtonY = 117;
        static const int m_ButtonW =  32;
        static const int m_ButtonH =  30;

    private:
        std::atomic<int> m_ProcState;

    private:
        // button status: 0 : normal
        //                1 : over
        //                2 : pressed
        int m_ButtonState;

    private:
        std::vector<LoadProc> m_LoadProcV;

    private:
        std::mutex                m_Lock;
        std::vector<MessageEntry> m_MessageRecord;

    private:
        TTF_Font    *m_TTF;
        SDL_Texture *m_TextureV[2];

    public:
        InitView(size_t);

    public:
        ~InitView();

    private:
        void Proc();
        void Load();
        void Draw();
        void ProcessEvent();

    private:
        void AddIVLog(int, const char *, ...);

    private:
        // we use nCurrIndex rather than percentage
        // since we may have other events not calling LoadDBN()
        template<typename T> bool LoadDBN(size_t nCurrIndex, const XMLConf *pXMLConf, T *pDBN, const char *szNodePath)
        {
            if(true
                    && nCurrIndex < m_LoadProcV.size()
                    && pXMLConf
                    && pDBN
                    && szNodePath){

                auto stArray = [this, nCurrIndex]() -> std::array<size_t, 2>
                {
                    std::array<size_t, 2> stArray {{0, 0}};
                    for(size_t nIndex = 0; nIndex < m_LoadProcV.size(); ++nIndex){
                        auto nWeight = m_LoadProcV[nIndex].Weight ? m_LoadProcV[nIndex].Weight : 1;
                        stArray[0] += (nWeight);
                        stArray[1] += (nIndex <= nCurrIndex ? nWeight : 0);
                    }
                    return stArray;
                }();

                auto nPercent = (int)(std::lround(stArray[1] * 100.0 / (0.1 + stArray[0])));
                AddIVLog(LOGIV_INFO, "[%03d%%]Loading %s", nPercent, szNodePath);
                if(auto pNode = pXMLConf->GetXMLNode(szNodePath)){
                    if(auto szPath = pNode->GetText()){
                        if(pDBN->Load(szPath)){
                            AddIVLog(LOGIV_INFO, "[%03d%%]Loading %s done", nPercent, szNodePath);
                            return true;
                        }else{
                            AddIVLog(LOGIV_WARNING, "[%03d%%]Loading %s failed: %s", nPercent, szNodePath, szPath);
                            return false;
                        }
                    }else{
                        AddIVLog(LOGIV_WARNING, "[%03d%%]Loading %s failed: Invalid text node in configuration", nPercent, szNodePath);
                        return false;
                    }
                }else{
                    AddIVLog(LOGIV_WARNING, "[%03d%%]Loading %s failed: No text node found in configuration", nPercent, szNodePath);
                    return false;
                }
            }
            AddIVLog(LOGIV_WARNING, "[%%---]Loading parameters invalid for LoadDBN(%d, %p, %p, %p)", (int)(nCurrIndex), pXMLConf, pDBN, szNodePath);
            return false;
        }
};
