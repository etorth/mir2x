// a seperate procedure to load all resource at beginning
// this procedure has its own event loop and two threads
//
// thread-1: event loop for a small window to view what's loading
// thread-2: loading the resource
//
// requirement:
// 1. SDL
// 2.

#pragma once
#include <atomic>
#include <array>
#include <mutex>
#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>
#include <SDL2/SDL.h>
#include <functional>
#include "bevent.hpp"
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

        struct LogEntry
        {
            int type = 0;
            std::string log;
            SDL_Texture *texture = nullptr;
        };

    private:
        static constexpr int m_buttonX = 345;
        static constexpr int m_buttonY = 117;
        static constexpr int m_buttonW =  32;
        static constexpr int m_buttonH =  30;

    private:
        std::atomic<bool> m_hasError{false};
        std::atomic<size_t> m_doneWeight{0};

    private:
        int m_buttonState = BEVENT_OFF;

    private:
        const uint8_t m_fontSize;

    private:
        std::vector<std::tuple<size_t, std::function<void(size_t)>>> m_taskList;

    private:
        std::mutex m_lock;
        std::vector<LogEntry> m_logSink;

    private:
        SDL_Texture *m_boardTexture  = nullptr;
        SDL_Texture *m_buttonTexture = nullptr;

    public:
        InitView(uint8_t);

    public:
        ~InitView();

    private:
        void draw();
        void processEventDefault();

    private:
        void addIVLog(int, const char *, ...);

    private:
        int donePercent() const
        {
            size_t taskWeigthSum = 0;
            for(const auto &[weight, task]: m_taskList){
                fflassert(weight);
                taskWeigthSum += weight;
            }
            return to_d(std::lround(to_f(m_doneWeight.load()) * 100.0 / taskWeigthSum));
        }

    private:
        template<typename T> void loadDB(size_t taskWeight, const XMLConf *xmlConfPtr, T *dbPtr, const char *nodePath)
        {
            fflassert(taskWeight);
            fflassert(xmlConfPtr);
            fflassert(dbPtr);
            fflassert(str_haschar(nodePath));

            if(m_hasError){
                return;
            }

            bool hasError = true;
            addIVLog(LOGIV_INFO, "[%03d%%]Loading %s", donePercent(), nodePath);

            if(auto nodePtr = xmlConfPtr->getXMLNode(nodePath)){
                if(auto dbPath = nodePtr->GetText()){
                    if(dbPtr->load(dbPath)){
                        hasError = false;
                        m_doneWeight += taskWeight;
                        addIVLog(LOGIV_INFO, "[%03d%%]Loading %s done", donePercent(), nodePath);
                    }
                    else{
                        addIVLog(LOGIV_WARNING, "[%03d%%]Loading %s failed: %s", donePercent(), nodePath, dbPath);
                    }
                }
                else{
                    addIVLog(LOGIV_WARNING, "[%03d%%]Loading %s failed: Invalid node in configuration", donePercent(), nodePath);
                }
            }
            else{
                addIVLog(LOGIV_WARNING, "[%03d%%]Loading %s failed: No node found in configuration", donePercent(), nodePath);
            }

            if(hasError){
                m_hasError = true;
            }
        }
};
