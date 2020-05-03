-- =====================================================================================
--
--       Filename: default.lua
--        Created: 08/31/2015 08:52:57 PM
--    Description: lua 5.3
--
--        Version: 1.0
--       Revision: none
--       Compiler: lua
--
--         Author: ANHONG
--          Email: anhonghe@gmail.com
--   Organization: USTC
--
-- =====================================================================================

event_handler = 
{
    ["init"] = function(uid)
        sayXML(string.format(
        [[
            <layout>                                                                
                <par>客官你好，我是%s，欢迎来到传奇旧时光！<emoji id="0"/></par>  
                <par>有什么可以为你效劳的吗？</par>                                 
                <par></par>                                                         
                <par><event id="event_1">如何快速升级</event></par>                       
                <par><event id="close">关闭</event></par>                           
            </layout>                                                               
        ]], getSelfName()))
    end,

    ["event_1"] = function(uid)
        sayXML(
        [[
            <layout>                                        
                <par>多多上线打怪升级！<emoji id="1"/></par>
                <par><event id="close">关闭</event></par>   
            </layout>                                       
        ]])
    end,
}

function main()
    local event, uid = waitEvent()
    event_handler[event](uid)
end
