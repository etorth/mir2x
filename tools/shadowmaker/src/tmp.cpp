{
    m_ValidAnimationBrowser->clear();
    
    // m_AnimationCount = nMaxIndex / 1000 + 1;
    m_AnimationCount = 9;
    for(int nSet = 0; nSet < m_AnimationCount; ++nSet){
        int nStartIndex = nSet * 3000 + 1;
        for(int nState = 0; nState < 33; ++nState){
            nStartIndex += (nState * 80);
            extern WilImagePackage g_WilImagePackage[2];
            extern const char *g_StatusNameList[];
            if(true
                    && g_WilImagePackage[0].SetIndex(nStartIndex)
                    && g_WilImagePackage[0].CurrentImageValid()
              ){
                char szInfo[512];
                std::sprintf(szInfo,
                        "Set/State: (%d:%02d)        %s", nSet, nState, g_StatusNameList[nState]);
                m_ValidAnimationBrowser->add(szInfo, (void *)nSet);
            }else{
                char szInfo[512];
                std::sprintf(szInfo,
                        "Set/State: (%d:%02d) (Lost) %s", nSet, nState, g_StatusNameList[nState]);
                m_ValidAnimationBrowser->add(szInfo, (void *)nSet);
            }


        }
    }
}
