/*
 * =====================================================================================
 *
 *       Filename: processnetevent.cpp
 *        Created: 01/23/2016 05:22:15
 *  Last Modified: 01/23/2016 05:23:20
 *
 *    Description: 
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

// 1. 31-bits nCode server as /HC/SubHC/DataLen/, nData1 and pData2 server as content
// 2. network interface:

//      take HC;
//      if(HC != 0){
//          this is a short message without content;
//      }else{
//              there is consequential message;
//              take SubHC;
//              if(SubHC == 0){
//                  take streaming ends up with '\0';
//              }else{
//                  take data with Length SubHC;
//              }
//          }
//      }

// for game logic:
// 
//      MessageID = MessageID2 << 8 + MessageID1
//
//      1. All messages without content should be:
//          MessageID2 = 0;
//          MessageID1 in 1 ~ 255

//      2. All messages with content should be:
//          MessageID2 != 0;
//          MessageID1 in 0 ~ 255
//
// How to send one message to network:
//
//          MessageID1 = (uint8_t)(MessageID & 0X00FF);
//          MessageID2 = (uint8_t)(MessageID & 0XFF00 >> 8);
//          
//          send MessageID1;
//          if(MessageID1 != 0){
//              // there is content and we need to send more;
//              send content.size();
//              send MessageID2;
//              send content;
//          }

// How to interpret message to game
//      take HC;
//      MessageID1 = HC;
//      if(HC != 0){
//          // this is a short message without content;
//          MessageID2 = 0;
//          Event.code = MessageID2 << 8 + MessageID1;
//          push Event to SDL;
//      }else{
//          // there is consequential message;
//          take SubHC;
//          DataLength = SubHC;
//          take MessageID2;
//          
//          if(DataLength == 0){
//              recveive stream until '\0' appears;
//              Event.Data2 = new uint8_t[stream.size()];
//              copy stream to Event.Data2;
//              
}
//              
//              if(SubHC == 0){
//                  take streaming ends up with '\0';
//              }else{
//                  take data with Length SubHC;
//              }
//          }
//      }



void Game::ProcessEvent(uint32_t nCode, uint64_t nData1, uint8_t *pData2)
{
    uint8_t B1InCode = (uint8_t)((nCode & 0X000000FF) >>  0);
    uint8_t B2InCode = (uint8_t)((nCode & 0X0000FF00) >>  8);
    uint8_t B3InCode = (uint8_t)((nCode & 0X00FF0000) >> 16);
    
    uint8_t chHC     = B1InCode;
    uint8_t chSubHC  = B2InCode;
    size_t  nDataLen = (size_t)B3InCode;
    
    switch(((uint16_t)(chHC) << 8) + chSubHC){
        case MSG_PING:      NEOnPing((Uint32)nData1); break;
        case MSG_PLAYERNUM: NEOnPlayerNum(nData1);    break;
        default: break;
    }
}
