// AddLog(int nLogType, const char *szMessage)
{
    std::time_t stRes         = std::time(nullptr);
    std::string szFullMessage = std::asctime(std::localtime(&stRes));
    szFullMessage.back() = ' ';
    switch(nLogType){
        case 0:
            szFullMessage = "@C71"  + szFullMessage + "  " + szMessage;
            break;
        case 1:
            szFullMessage = "@C128" + szFullMessage + "  " + szMessage;
            break;
        default:
            szFullMessage = "@C230" + szFullMessage + "  " + szMessage;
            break;
    }
    m_Browser->add(szFullMessage.c_str());
}
