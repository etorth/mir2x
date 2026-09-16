local dialog = require('include.dialog')
local dq = {}

local QUEST_NAME = '今日任务'
local MAX_DAILY_ROUNDS = 20

local DBVAR_ACTIVE_TARGET   = 'dailyquest.activeTarget'
local DBVAR_COMPLETED_DAY   = 'dailyquest.completedDay'
local DBVAR_COMPLETED_COUNT = 'dailyquest.completedCount'

local targetNPCList <const> =
{
    {name = '王大人',         area = '比奇城',           role = '',         map = '比奇县_0',          npc = '王大人_1',         location = '比奇城<t color="RED">390，397</t>附近'             },
    {name = '苏百花',         area = '比奇城',           role = '',         map = '比奇县_0',          npc = '苏百花_1',         location = '比奇城<t color="RED">483，405</t>附近'             },
    {name = '太俊',           area = '比奇城',           role = '材料商人', map = '比奇县_0',          npc = '太俊_1',           location = '比奇县<t color="RED">417，405</t>附近'             },
    {name = '金氏',           area = '比奇城',           role = '肉店',     map = '比奇县_0',          npc = '金氏_1',           location = '比奇县<t color="RED">446，405</t>附近'             },
    {name = '肉店老板',       area = '比奇城',           role = '',         map = '比奇县_0',          npc = '肉店老板_1',       location = '比奇县<t color="RED">441，404</t>附近'             },
    {name = '老张',           area = '比奇城',           role = '武器商',   map = '比奇县_0',          npc = '老张_1',           location = '比奇县<t color="RED">423，395</t>附近'             },
    {name = '阿康',           area = '比奇城',           role = '铁匠铺',   map = '比奇县_0',          npc = '阿康_1',           location = '比奇县<t color="RED">402，356</t>附近'             },
    {name = '怡美',           area = '比奇城',           role = '棉布店',   map = '比奇县_0',          npc = '怡美_1',           location = '比奇县<t color="RED">480，407</t>附近'             },
    {name = '慧媛',           area = '比奇城',           role = '鞋子商',   map = '比奇县_0',          npc = '慧媛_1',           location = '比奇县<t color="RED">478，407</t>附近'             },
    {name = '药店老板',       area = '比奇城',           role = '中药商',   map = '比奇县_0',          npc = '药店老板_1',       location = '比奇县<t color="RED">397，363</t>附近'             },
    {name = '恩英',           area = '比奇城',           role = '中药商',   map = '比奇县_0',          npc = '恩英_1',           location = '比奇县<t color="RED">486，414</t>附近'             },
    {name = '药剂师',         area = '比奇城',           role = '药铺的',   map = '比奇县_0',          npc = '药剂师_1',         location = '比奇县<t color="RED">412，410</t>附近'             },
    {name = '店员',           area = '比奇城',           role = '书店',     map = '比奇县_0',          npc = '店员_1',           location = '比奇县<t color="RED">470，424</t>附近'             },
    {name = '客栈店员',       area = '比奇城',           role = '仓库',     map = '比奇县_0',          npc = '客栈店员_1',       location = '比奇县<t color="RED">425，361</t>附近'             },
    {name = '杂货商',         area = '比奇城',           role = '杂货店',   map = '比奇县_0',          npc = '杂货商_1',         location = '比奇县<t color="RED">450，413</t>附近'             },
    {name = '恩实',           area = '比奇城',           role = '饰品店',   map = '比奇县_0',          npc = '恩实_1',           location = '比奇县<t color="RED">414，349</t>附近'             },
    {name = '世玉',           area = '比奇城',           role = '彩卷商',   map = '比奇县_0',          npc = '世玉_1',           location = '比奇县<t color="RED">440，341</t>附近'             },
    {name = '酒娘',           area = '比奇城',           role = '',         map = '比奇县_0',          npc = '酒娘_1',           location = '比奇县<t color="RED">420，431</t>附近'             },
    {name = '图书管理员',     area = '比奇城',           role = '',         map = '比奇县_0',          npc = '图书管理员_1',     location = '比奇县<t color="RED">473，428</t>附近'             },
    {name = '比奇道长',       area = '比奇城',           role = '',         map = '比奇县_0',          npc = '比奇道长_1',       location = '比奇县<t color="RED">449，433</t>附近'             },
    {name = '老生',           area = '比奇城',           role = '夜市商人', map = '比奇县_0',          npc = '老生_1',           location = '比奇县<t color="RED">452，297</t>附近'             },
    {name = '美容师',         area = '比奇城',           role = '',         map = '比奇县_0',          npc = '美容师_1',         location = '比奇县<t color="RED">462，420</t>附近'             },
    {name = '阿辉',           area = '边境城市',         role = '材料商人', map = '边境城市_01',       npc = '阿辉_1',           location = '边境城市<t color="RED">431，307</t>附近'           },
    {name = '德秀',           area = '边境城市',         role = '铁匠铺',   map = '边境城市_01',       npc = '德秀_1',           location = '边境城市<t color="RED">459，279</t>附近'           },
    {name = '顺子',           area = '边境城市',         role = '棉布店',   map = '边境城市_01',       npc = '顺子_1',           location = '边境城市<t color="RED">442，296</t>附近'           },
    {name = '尹老人',         area = '边境城市',         role = '中药商',   map = '边境城市_01',       npc = '尹老人_1',         location = '边境城市<t color="RED">434，303</t>附近'           },
    {name = '老余',           area = '边境城市',         role = '书店',     map = '边境城市_01',       npc = '老余_1',           location = '边境城市<t color="RED">445，319</t>附近'           },
    {name = '赵老头',         area = '边境城市',         role = '仓库',     map = '边境城市_01',       npc = '赵老头_1',         location = '边境城市<t color="RED">471，279</t>附近'           },
    {name = '地摊商人',       area = '边境城市',         role = '杂货商',   map = '边境城市_01',       npc = '地摊商人_1',       location = '边境城市<t color="RED">433，282</t>附近'           },
    {name = '晓丽',           area = '边境城市',         role = '饰品商',   map = '边境城市_01',       npc = '晓丽_1',           location = '边境城市<t color="RED">444，270</t>附近'           },
    {name = '阿勋',           area = '银杏山谷',         role = '材料商人', map = '银杏山谷_02',       npc = '阿勋_1',           location = '银杏山谷<t color="RED">269，186</t>附近'           },
    {name = '许氏',           area = '银杏山谷',         role = '肉店',     map = '银杏山谷_02',       npc = '许氏_1',           location = '银杏山谷<t color="RED">228，194</t>附近'           },
    {name = '铁匠师傅',       area = '银杏山谷',         role = '铁匠铺',   map = '银杏山谷_02',       npc = '铁匠师傅_1',       location = '银杏山谷<t color="RED">284，197</t>附近'           },
    {name = '布店晓芙',       area = '银杏山谷',         role = '棉布店',   map = '银杏山谷_02',       npc = '布店晓芙_1',       location = '银杏山谷<t color="RED">260，177</t>附近'           },
    {name = '许中医',         area = '银杏山谷',         role = '中药商',   map = '银杏山谷_02',       npc = '许中医_1',         location = '银杏山谷<t color="RED">273，189</t>附近'           },
    {name = '药剂师',         area = '银杏山谷',         role = '药铺的',   map = '银杏山谷_02',       npc = '药剂师_1',         location = '银杏山谷<t color="RED">272，189</t>附近'           },
    {name = '才石',           area = '银杏山谷',         role = '书店',     map = '银杏山谷_02',       npc = '才石_1',           location = '银杏山谷<t color="RED">233，222</t>附近'           },
    {name = '阿雯',           area = '银杏山谷',         role = '仓库',     map = '银杏山谷_02',       npc = '阿雯_1',           location = '银杏山谷<t color="RED">250，190</t>附近'           },
    {name = '地摊老周',       area = '银杏山谷',         role = '杂货商',   map = '银杏山谷_02',       npc = '地摊老周_1',       location = '银杏山谷<t color="RED">236，205</t>附近'           },
    {name = '晓兰',           area = '银杏山谷',         role = '饰品商',   map = '银杏山谷_02',       npc = '晓兰_1',           location = '银杏山谷<t color="RED">275，222</t>附近'           },
    {name = '南宫小姐',       area = '银杏山谷',         role = '',         map = '银杏山谷_02',       npc = '南宫小姐_1',       location = '银杏山谷<t color="RED">264，201</t>附近'           },
    {name = '化天先生',       area = '银杏山谷',         role = '',         map = '银杏山谷_02',       npc = '化天先生_1',       location = '银杏山谷<t color="RED">353，212</t>附近'           },
    {name = '霹雳尊者',       area = '银杏山谷',         role = '',         map = '银杏山谷_02',       npc = '霹雳尊者_1',       location = '银杏山谷<t color="RED">265，145</t>附近'           },
    {name = '天星',           area = '道馆',             role = '材料商人', map = '道馆_1',            npc = '天星_1',           location = '道馆<t color="RED">386，119</t>附近的建筑物内'     },
    {name = '阿潘',           area = '道馆',             role = '武器商',   map = '武器仓库_1_001',    npc = '阿潘_1',           location = '道馆<t color="RED">429，120</t>附近的建筑物内'     },
    {name = '铁匠',           area = '道馆',             role = '武器商',   map = '道馆_1',            npc = '铁匠_1',           location = '道馆<t color="RED">426，117</t>附近'               },
    {name = '阿浩',           area = '道馆',             role = '棉布店',   map = '洗衣住居_1_003',    npc = '阿浩_1',           location = '道馆<t color="RED">415，133</t>附近的建筑物内'     },
    {name = '梁生',           area = '道馆',             role = '棉布店',   map = '道馆_1',            npc = '梁生_1',           location = '道馆<t color="RED">412，133</t>附近'               },
    {name = '老药',           area = '道馆',             role = '中药商',   map = '药剂师住居_1_006',  npc = '老药_1',           location = '道馆<t color="RED">388，113</t>附近的建筑物内'     },
    {name = '药中',           area = '道馆',             role = '中药商',   map = '道馆_1',            npc = '药中_1',           location = '道馆<t color="RED">393，113</t>附近'               },
    {name = '禄英',           area = '道馆',             role = '药铺的',   map = '药剂师住居_1_006',  npc = '禄英_1',           location = '道馆<t color="RED">388，113</t>附近的建筑物内'     },
    {name = '药神',           area = '道馆',             role = '药铺的',   map = '道馆_1',            npc = '药神_1',           location = '道馆<t color="RED">387，118</t>附近'               },
    {name = '书神',           area = '道馆',             role = '书店',     map = '书房_1_004',        npc = '书神_1',           location = '道馆<t color="RED">415，96</t>附近的建筑物内'      },
    {name = '书鬼',           area = '道馆',             role = '书店',     map = '道馆_1',            npc = '书鬼_1',           location = '道馆<t color="RED">418，96</t>附近'                },
    {name = '阿天',           area = '道馆',             role = '仓库',     map = '道馆_1',            npc = '阿天_1',           location = '道馆<t color="RED">396，116</t>附近'               },
    {name = '大老板',         area = '道馆',             role = '杂货商',   map = '仓库_1_007',        npc = '大老板_1',         location = '道馆<t color="RED">394，169</t>附近的建筑物内'     },
    {name = '小老板',         area = '道馆',             role = '杂货商',   map = '道馆_1',            npc = '小老板_1',         location = '道馆<t color="RED">391，167</t>附近'               },
    {name = '峰儿',           area = '道馆',             role = '饰品商',   map = '物品研究所_1_005',  npc = '峰儿_1',           location = '道馆<t color="RED">367，134</t>附近的建筑物内'     },
    {name = '钱老板',         area = '道馆前边城市',     role = '肉店',     map = '道馆_1',            npc = '钱老板_1',         location = '道馆<t color="RED">366，182</t>附近'               },
    {name = '华玉',           area = '道馆前边城市',     role = '中药商',   map = '道馆_1',            npc = '华玉_1',           location = '道馆<t color="RED">363，176</t>附近'               },
    {name = '蛇谷老矿夫',     area = '蛇谷城市',         role = '',         map = '毒蛇山谷_2',        npc = '蛇谷老矿夫_1',     location = '毒蛇山谷<t color="RED">360，209</t>附近'           },
    {name = '蛇谷老太',       area = '蛇谷城市',         role = '',         map = '毒蛇山谷_2',        npc = '蛇谷老太_1',       location = '毒蛇山谷<t color="RED">337，223</t>附近'           },
    {name = '阿福',           area = '蛇谷城市',         role = '材料商人', map = '毒蛇山谷_2',        npc = '阿福_1',           location = '毒蛇山谷<t color="RED">331，225</t>附近'           },
    {name = '金莲',           area = '蛇谷城市',         role = '棉布店',   map = '毒蛇山谷_2',        npc = '金莲_1',           location = '毒蛇山谷<t color="RED">342，235</t>附近'           },
    {name = '金中医',         area = '蛇谷城市',         role = '中药商',   map = '毒蛇山谷_2',        npc = '金中医_1',         location = '毒蛇山谷<t color="RED">334，224</t>附近'           },
    {name = '客栈保管员',     area = '蛇谷城市',         role = '仓库',     map = '毒蛇山谷_2',        npc = '客栈保管员_1',     location = '毒蛇山谷<t color="RED">346，234</t>附近'           },
    {name = '蛇谷老人',       area = '蛇谷城市',         role = '',         map = '毒蛇山谷_2',        npc = '蛇谷老人_1',       location = '毒蛇山谷<t color="RED">343，219</t>附近'           },
    {name = '断乔先生',       area = '蛇谷城市',         role = '',         map = '毒蛇山谷_2',        npc = '断乔先生_1',       location = '毒蛇山谷<t color="RED">347，193</t>附近'           },
    {name = '矿山老人',       area = '蛇谷矿山',         role = '',         map = '北部矿山入口_D431', npc = '矿山老人_1',       location = '矿山1层<t color="RED">34，368</t>附近'             },
    {name = '俞公',           area = '沙巴克城',         role = '材料商人', map = '沙巴克城_3',        npc = '俞公_1',           location = '沙巴克城<t color="RED">203，197</t>附近'           },
    {name = '老胡',           area = '沙巴克城',         role = '铁匠铺',   map = '沙巴克城_3',        npc = '老胡_1',           location = '沙巴克城<t color="RED">195，183</t>附近'           },
    {name = '晶晶',           area = '沙巴克城',         role = '中药商',   map = '沙巴克城_3',        npc = '晶晶_1',           location = '沙巴克城<t color="RED">204，196</t>附近'           },
    {name = '惠莲',           area = '沙巴克城',         role = '仓库',     map = '沙巴克城_3',        npc = '惠莲_1',           location = '沙巴克城<t color="RED">214，128</t>附近'           },
    {name = '多多',           area = '沙巴克城',         role = '手镯商',   map = '沙巴克城_3',        npc = '多多_1',           location = '沙巴克城<t color="RED">211，183</t>附近'           },
    {name = '石宇',           area = '绿洲',             role = '材料商人', map = '中药商_4_003',      npc = '石宇_1',           location = '绿洲<t color="RED">433，72</t>附近的建筑物内'      },
    {name = '润京',           area = '绿洲',             role = '材料商人', map = '绿洲_4',            npc = '润京_1',           location = '绿洲<t color="RED">437，71</t>附近'                },
    {name = '屠夫',           area = '绿洲',             role = '肉店',     map = '绿洲_4',            npc = '屠夫_1',           location = '绿洲<t color="RED">438，52</t>附近'                },
    {name = '王铁匠',         area = '绿洲',             role = '铁匠铺',   map = '武器店_4_001',      npc = '王铁匠_1',         location = '绿洲<t color="RED">449，73</t>附近的建筑物内'      },
    {name = '丁铁匠',         area = '绿洲',             role = '铁匠铺',   map = '绿洲_4',            npc = '丁铁匠_1',         location = '绿洲<t color="RED">448，76</t>附近'                },
    {name = '织女',           area = '绿洲',             role = '棉布店',   map = '布匹店_4_004',      npc = '织女_1',           location = '绿洲<t color="RED">462，89</t>附近的建筑物内'      },
    {name = '布商女店员',     area = '绿洲',             role = '棉布店',   map = '绿洲_4',            npc = '布商女店员_1',     location = '绿洲<t color="RED">462，91</t>附近'                },
    {name = '白老中医',       area = '绿洲',             role = '中药商',   map = '中药商_4_003',      npc = '白老中医_1',       location = '绿洲<t color="RED">433，72</t>附近的建筑物内'      },
    {name = '医仙',           area = '绿洲',             role = '中药商',   map = '绿洲_4',            npc = '医仙_1',           location = '绿洲<t color="RED">430，75</t>附近'                },
    {name = '绿洲仓库管家',   area = '绿洲',             role = '仓库',     map = '中药商_4_003',      npc = '绿洲仓库管家_1',   location = '绿洲<t color="RED">433，72</t>附近的建筑物内'      },
    {name = '绿洲仓库保管员', area = '绿洲',             role = '仓库',     map = '绿洲_4',            npc = '绿洲仓库保管员_1', location = '绿洲<t color="RED">438，68</t>附近'                },
    {name = '洪老板',         area = '绿洲',             role = '杂货商',   map = '绿洲_4',            npc = '洪老板_1',         location = '绿洲<t color="RED">470，67</t>附近'                },
    {name = '梅山侠',         area = '绿洲',             role = '法师',     map = '占卜屋_4_005',      npc = '梅山侠_1',         location = '绿洲<t color="RED">476，57</t>附近的建筑物内'      },
    {name = '峨嵋侠',         area = '绿洲',             role = '饰品商',   map = '绿洲_4',            npc = '峨嵋侠_1',         location = '绿洲<t color="RED">475，55</t>附近'                },
    {name = '唯我独尊',       area = '绿洲',             role = '',         map = '绿洲_4',            npc = '唯我独尊_1',       location = '绿洲<t color="RED">426，97</t>附近的建筑物内'      },
    {name = '阿宋',           area = '沙漠土城',         role = '材料商人', map = '沙漠土城_5',        npc = '阿宋_1',           location = '沙漠土城<t color="RED">197，235</t>附近'           },
    {name = '黄老板',         area = '沙漠土城',         role = '肉店',     map = '沙漠土城_5',        npc = '黄老板_1',         location = '沙漠土城<t color="RED">200，249</t>附近'           },
    {name = '勇汉',           area = '沙漠土城',         role = '铁匠铺',   map = '武器商_5_002',      npc = '勇汉_1',           location = '沙漠土城<t color="RED">190，249</t>附近的建筑物内' },
    {name = '铁汉',           area = '沙漠土城',         role = '铁匠铺',   map = '沙漠土城_5',        npc = '铁汉_1',           location = '沙漠土城<t color="RED">189，252</t>附近'           },
    {name = '晓敏',           area = '沙漠土城',         role = '棉布店',   map = '服饰店_5_004',      npc = '晓敏_1',           location = '沙漠土城<t color="RED">179，278</t>附近的建筑物内' },
    {name = '喜儿',           area = '沙漠土城',         role = '棉布店',   map = '沙漠土城_5',        npc = '喜儿_1',           location = '沙漠土城<t color="RED">177，281</t>附近'           },
    {name = '润真',           area = '沙漠土城',         role = '鞋子商',   map = '沙漠土城_5',        npc = '润真_1',           location = '沙漠土城<t color="RED">181，277</t>附近'           },
    {name = '医神',           area = '沙漠土城',         role = '中药商',   map = '药房_5_003',        npc = '医神_1',           location = '沙漠土城<t color="RED">193，231</t>附近的建筑物内' },
    {name = '药郎',           area = '沙漠土城',         role = '中药商',   map = '沙漠土城_5',        npc = '药郎_1',           location = '沙漠土城<t color="RED">200，237</t>附近'           },
    {name = '沙漠仓库管家',   area = '沙漠土城',         role = '仓库',     map = '仓库_5_006',        npc = '沙漠仓库管家_1',   location = '沙漠土城<t color="RED">159，267</t>附近的建筑物内' },
    {name = '沙漠仓库小管家', area = '沙漠土城',         role = '仓库',     map = '沙漠土城_5',        npc = '沙漠仓库小管家_1', location = '沙漠土城<t color="RED">171，265</t>附近'           },
    {name = '杂货商老李',     area = '沙漠土城',         role = '杂货商',   map = '沙漠土城_5',        npc = '杂货商老李_1',     location = '沙漠土城<t color="RED">192，232</t>附近'           },
    {name = '雪儿',           area = '沙漠土城',         role = '饰品商',   map = '饰品店_5_005',      npc = '雪儿_1',           location = '沙漠土城<t color="RED">203，241</t>附近的建筑物内' },
    {name = '倩儿',           area = '沙漠土城',         role = '饰品商',   map = '沙漠土城_5',        npc = '倩儿_1',           location = '沙漠土城<t color="RED">208，246</t>附近'           },
    {name = '芙蓉仙子',       area = '沙漠土城',         role = '',         map = '沙漠土城_5',        npc = '芙蓉仙子_1',       location = '沙漠土城<t color="RED">230，188</t>附近的建筑物内' },
    {name = '小秀玉女',       area = '沙漠土城',         role = '',         map = '沙漠土城_5',        npc = '小秀玉女_1',       location = '沙漠土城<t color="RED">199，203</t>附近的建筑物内' },
    {name = '阿贵',           area = '盟重土城',         role = '材料商人', map = '盟重县_74',         npc = '阿贵_1',           location = '盟重土城<t color="RED">287，290</t>附近'           },
    {name = '英龙',           area = '盟重土城',         role = '铁匠铺',   map = '盟重县_74',         npc = '英龙_1',           location = '盟重土城<t color="RED">354，297</t>附近'           },
    {name = '晓薇',           area = '盟重土城',         role = '棉布店',   map = '盟重县_74',         npc = '晓薇_1',           location = '盟重土城<t color="RED">324，277</t>附近'           },
    {name = '凌峰',           area = '盟重土城',         role = '中药商',   map = '盟重县_74',         npc = '凌峰_1',           location = '盟重土城<t color="RED">289，289</t>附近'           },
    {name = '丘叔',           area = '盟重土城',         role = '仓库',     map = '盟重县_74',         npc = '丘叔_1',           location = '盟重土城<t color="RED">323，310</t>附近的建筑物内' },
    {name = '龙叔',           area = '盟重土城',         role = '杂货商',   map = '盟重县_74',         npc = '龙叔_1',           location = '盟重土城<t color="RED">293，278</t>附近'           },
    {name = '善颖',           area = '盟重土城',         role = '饰品商',   map = '盟重县_74',         npc = '善颖_1',           location = '盟重土城<t color="RED">311，273</t>附近'           },
    {name = '盟重道长',       area = '盟重土城',         role = '',         map = '盟重县_74',         npc = '盟重道长_1',       location = '盟重土城<t color="RED">297，295</t>附近'           },
    {name = '雷翁断珠',       area = '盟重土城',         role = '',         map = '盟重县_74',         npc = '雷翁断珠_1',       location = '盟重土城<t color="RED">290，299</t>附近'           },
    {name = '泼皮',           area = '潘夜岛',           role = '材料商人', map = '潘夜岛_8',          npc = '泼皮_1',           location = '潘夜岛<t color="RED">271，222</t>附近'             },
    {name = '晓洋',           area = '潘夜岛',           role = '棉布店',   map = '潘夜岛_8',          npc = '晓洋_1',           location = '潘夜岛<t color="RED">249，257</t>附近'             },
    {name = '成赫',           area = '潘夜岛',           role = '中药商',   map = '潘夜岛_8',          npc = '成赫_1',           location = '潘夜岛<t color="RED">282，224</t>附近'             },
    {name = '满春',           area = '潘夜岛',           role = '仓库',     map = '潘夜岛_8',          npc = '满春_1',           location = '潘夜岛<t color="RED">231，260</t>附近的建筑物内'   },
    {name = '中叔',           area = '潘夜岛',           role = '杂货商',   map = '潘夜岛_8',          npc = '中叔_1',           location = '潘夜岛<t color="RED">253，291</t>附近'             },
    {name = '晓华',           area = '潘夜岛',           role = '饰品商',   map = '潘夜岛_8',          npc = '晓华_1',           location = '潘夜岛<t color="RED">266，285</t>附近'             },
    {name = '阿全',           area = '南部蚂蚁洞穴入口', role = '杂货商',   map = '沙漠_6',            npc = '阿全_1',           location = '沙漠<t color="RED">284，723</t>附近'               },
    {name = '洞穴保管员',     area = '半兽洞穴',         role = '仓库',     map = '半兽洞穴2层_D002',  npc = '洞穴保管员_1',     location = '半兽洞穴2层<t color="RED">174，216</t>附近'        },
    {name = '奸商欧阳',       area = '半兽洞穴',         role = '杂货商',   map = '奸商_DM001',        npc = '奸商欧阳_1',       location = '半兽洞穴2层<t color="RED">191，231</t>附近的洞窟内'},
}

local rewardList <const> =
{
    {kind = 'gold',                        count = 1000, description = '<t color="RED">1000金币</t>'       },
    {kind = 'exp',                         count = 1000, description = '<t color="RED">1000点经验</t>'     },
    {kind = 'item', item = '金创药（小）', count = 5,    description = '<t color="RED">5瓶金创药（小）</t>'},
    {kind = 'item', item = '魔法药（小）', count = 5,    description = '<t color="RED">5瓶魔法药（小）</t>'},
    {kind = 'item', item = '太阳水',       count = 2,    description = '<t color="RED">2瓶太阳水</t>'      },
    {kind = 'item', item = '回城卷',       count = 1,    description = '<t color="RED">1张回城卷</t>'      },
}

local impatientTalk <const> =
{
    '怎么又是你，还没办好吗<t wrap="0">···</t>',
    '这事不是说过了吗<t wrap="0">···</t>',
    '你还没找到人？动作快点<t wrap="0">···</t>',
    '你又来问？我都说过了<t wrap="0">···</t>',
}

local function addImpatientTalk(impatient, dialogTable)
    if impatient then
        table.insert(dialogTable, 1, impatientTalk[math.random(1, #impatientTalk)])
    end
    return dialogTable
end

local dialogTemplateList <const> =
{
    function(target, again)
        return addImpatientTalk(again,
        {
            string.format('这次事情去找一下在<t color="RED">%s</t>的%s<t color="RED">%s</t>吧。', target.area, target.role, target.name),
            string.format('%s也许在%s。', target.name, target.location),
        })
    end,

    function(target, again)
        return addImpatientTalk(again,
        {
            string.format('麻烦你去一趟<t color="RED">%s</t>，把消息带给%s<t color="RED">%s</t>。', target.area, target.role, target.name),
            string.format('你可以到%s找%s。', target.location, target.name),
        })
    end,

    function(target, again)
        return addImpatientTalk(again,
        {
            string.format('今天的事情要请%s<t color="RED">%s</t>帮忙，你去<t color="RED">%s</t>找一下吧。', target.role, target.name, target.area),
            string.format('%s通常在%s。', target.name, target.location),
        })
    end,

    function(target, again)
        return addImpatientTalk(again,
        {
            string.format('要请%s<t color="RED">%s</t>帮忙，你去<t color="RED">%s</t>找一下吧。', target.role, target.name, target.area),
            string.format('%s通常在%s。', target.name, target.location),
        })
    end,
}

local function getQuestProgress(playerUID)
    return uidRemoteCall(playerUID, DBVAR_ACTIVE_TARGET, DBVAR_COMPLETED_DAY, DBVAR_COMPLETED_COUNT,
    [[
        local DBVAR_ACTIVE_TARGET, DBVAR_COMPLETED_DAY, DBVAR_COMPLETED_COUNT = ...

        local completedCount = dbGetVar(DBVAR_COMPLETED_COUNT) or 0
        local completedDay = dbGetVar(DBVAR_COMPLETED_DAY)
        local currentDay = (launchTime() + uptime()) // (24 * 60 * 60)

        if completedDay ~= currentDay then
            completedCount = 0
            dbSetVar(DBVAR_COMPLETED_DAY, currentDay)
            dbSetVar(DBVAR_COMPLETED_COUNT, completedCount)
        end
        return dbGetVar(DBVAR_ACTIVE_TARGET), completedCount
    ]])
end

local function setActiveTarget(playerUID, targetIndex)
    uidRemoteCall(playerUID, DBVAR_ACTIVE_TARGET, targetIndex,
    [[
        local DBVAR_ACTIVE_TARGET, targetIndex = ...
        dbSetVar(DBVAR_ACTIVE_TARGET, targetIndex)
    ]])
end

local function registerTargetResponse(playerUID, targetIndex)
    local target = targetNPCList[targetIndex]
    local npcUID = getNPCharUID(target.map, target.npc)

    if not npcUID then
        fatalPrintf('Daily quest target not found: map=%s, npc=%s', target.map, target.npc)
    end

    uidRemoteCall(npcUID, playerUID, targetIndex, QUEST_NAME, MAX_DAILY_ROUNDS, DBVAR_ACTIVE_TARGET, DBVAR_COMPLETED_DAY, DBVAR_COMPLETED_COUNT, rewardList,
    [[
        local playerUID, targetIndex, QUEST_NAME, MAX_DAILY_ROUNDS, DBVAR_ACTIVE_TARGET, DBVAR_COMPLETED_DAY, DBVAR_COMPLETED_COUNT, rewardList = ...

        local dialog = require('include.dialog')
        local questPath = {SYS_EPUID, QUEST_NAME}

        setUIDQuestHandler(playerUID, QUEST_NAME,
        {
            [SYS_LABEL] = '送达消息',
            [SYS_ENTER] = function(uid, args)
                local completed, completedCount = uidRemoteCall(uid, targetIndex, MAX_DAILY_ROUNDS, DBVAR_ACTIVE_TARGET, DBVAR_COMPLETED_DAY, DBVAR_COMPLETED_COUNT,
                [=[
                    local targetIndex, MAX_DAILY_ROUNDS, DBVAR_ACTIVE_TARGET, DBVAR_COMPLETED_DAY, DBVAR_COMPLETED_COUNT = ...

                    local completedCount = dbGetVar(DBVAR_COMPLETED_COUNT) or 0
                    local completedDay = dbGetVar(DBVAR_COMPLETED_DAY)
                    local currentDay = (launchTime() + uptime()) // (24 * 60 * 60)

                    if completedDay ~= currentDay then
                        completedCount = 0
                        dbSetVar(DBVAR_COMPLETED_DAY, currentDay)
                        dbSetVar(DBVAR_COMPLETED_COUNT, completedCount)
                    end

                    if dbGetVar(DBVAR_ACTIVE_TARGET) ~= targetIndex or completedCount >= MAX_DAILY_ROUNDS then
                        return false, completedCount
                    end

                    dbRemoveVar(DBVAR_ACTIVE_TARGET)
                    dbSetVar(DBVAR_COMPLETED_COUNT, completedCount + 1)
                    return true, completedCount + 1
                ]=])

                deleteUIDQuestHandler(uid, QUEST_NAME)
                if not completed then
                    dialog.post(uid, questPath, '这件事情已经处理过了。',
                    dialog.link(SYS_EXIT, '结束'))
                    return
                end

                local rewardIndex = math.random(1, #rewardList)
                local reward = rewardList[rewardIndex]

                if reward.kind == 'exp' then
                    server.player.addExp(uid, reward.count)
                elseif reward.kind == 'gold' then
                    server.player.addItem(uid, SYS_GOLDNAME, reward.count)
                elseif reward.kind == 'item' then
                    server.player.addItem(uid, reward.item, reward.count)
                else
                    fatalPrintf('Invalid daily quest reward type: %s', tostring(reward.kind))
                end

                dialog.post(uid, questPath,
                {
                    string.format('谢谢你带来的消息，我已经知道了，这是给你的谢礼：%s。', reward.description),
                    string.format('今天已经完成<t color="red">%d</t>次任务，还可以完成<t color="red">%d</t>次。', completedCount, MAX_DAILY_ROUNDS - completedCount),
                },
                dialog.link(SYS_EXIT, '结束'))
            end,
        })
    ]])
end

function dq.setQuest(uid, args)
    assertType(uid, 'integer')
    assert(isPlayer(uid))

    local targetIndex, completedCount = getQuestProgress(uid)
    if completedCount >= MAX_DAILY_ROUNDS then
        dialog.post(uid, string.format('你今天已经完成<t color="RED">%d</t>次任务，明天再来吧。', MAX_DAILY_ROUNDS),
        dialog.link(SYS_EXIT, '结束', {close = true}))
        return
    end

    local again = false
    if targetIndex == nil then
        targetIndex = math.random(1, #targetNPCList)
        setActiveTarget(uid, targetIndex)
    elseif targetNPCList[targetIndex] == nil then
        fatalPrintf('Invalid active daily quest target: %s', tostring(targetIndex))
    else
        -- no quest description added intentionally
        -- so if player forgot the target name or location, they have come back and ask again
        again = true
    end

    -- always register the target NPC's response handler
    -- when the server restarted, the target NPC's in-memory handler was lost while the db-persisted active target survived
    registerTargetResponse(uid, targetIndex)

    dialog.post(uid, dialogTemplateList[math.random(1, #dialogTemplateList)](targetNPCList[targetIndex], again),
    dialog.link(SYS_EXIT, '结束', {close = true}))
end

return dq
