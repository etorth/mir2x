import os
import sys
import math
from PIL import Image
import statistics


def pngs_2_emoji(png_list):
    assert png_list

    img_width  = png_list[0].size[0]
    img_height = png_list[0].size[1]

    for png in png_list:
        assert img_width  == png.size[0]
        assert img_height == png.size[1]

    best_usage = 0
    best_width_count = 0

    for width_count in range(1, len(png_list) + 1):
        height_count = (len(png_list) + width_count - 1) // width_count
        usage = (len(png_list) * img_width * img_height) / max(width_count * img_width, height_count * img_height) ** 2

        if usage > best_usage:
            best_usage = usage
            best_width_count = width_count

    best_height_count = (len(png_list) + best_width_count - 1) // best_width_count
    merged_png = Image.new("RGBA", (best_width_count * img_width, best_height_count * img_height), (0, 0, 0, 0))

    index = 0
    for ih in range(best_height_count):
        for iw in range(best_width_count):
            if index >= len(png_list):
                break

            dst_x = img_width  * iw + int((img_width  - png_list[index].size[0]) / 2)
            dst_y = img_height * ih + int((img_height - png_list[index].size[1]) / 2)

            merged_png.paste(png_list[index], (dst_x, dst_y))
            index += 1

    return merged_png, len(png_list), img_width, img_height, 5


def convert_emoji_repo(repo_path, emoji_set, output_path):
    assert os.path.isdir(repo_path), repo_path
    assert emoji_set >= 0X00, emoji_set
    assert emoji_set <= 0XFF, emoji_set

    for emoji_index in range(0, 0XFFFF + 1):
        png_formated_path = repo_path + '/' + '%s_%s.png'
        if os.path.isfile(png_formated_path % (emoji_index, 1)):
            png_list = []
            emoji_frame = 1

            while os.path.isfile(png_formated_path % (emoji_index, emoji_frame)):
                png_list.append(Image.open(png_formated_path % (emoji_index, emoji_frame), 'r'))
                emoji_frame += 1

            # create name to load in zsdb
            # check client/src/emoticondb.hpp see how name get formated
            merged_png, frame_count, frame_width, frame_height, fps = pngs_2_emoji(png_list)
            merged_png.save("%s/%02X%04X00%02X%02X%04X%04X%04X.PNG" % (output_path, emoji_set, emoji_index, frame_count, fps, frame_width, frame_height, frame_height))


def main():
    # need to install pillow:
    # python3 -m pip install --upgrade Pillow

    # input: https://github.com/etorth/mhxy-icons.git
    # how pngs in above repo get created:
    # 1. use following hash codes to find gfx in 资源.wdf
    #    but keep mind some indices are missing, indices changes between different versions of mhxy-binary
    #    this table uses release: http://xyq.gdl.netease.com/MHXY-JD-3.0.169.exe
    #
    #       0417C932 	 #52  	 痛哭，好伤心啊！
    #       09574327 	 #109 	 浪漫
    #       0E658C4C 	 #58  	 ZZZZZZZZZZ……
    #       107CF5F3 	 #97  	 一见钟情
    #       11729962 	 #92  	 Yeah!
    #       11C5EA40 	 #38  	 见到你好高兴！
    #       12BE1C3E 	 #59  	 XX山泉有点甜。
    #       1500E768 	 #19  	 我是MM
    #       15CA26D9 	 #85  	 看我怎么收拾你
    #       19CA9706 	 #54  	 我生气啦！
    #       1B1B8326 	 #14  	 发愁啊～～
    #       1C0BCE22 	 #113 	 俯卧撑
    #       1C7C95C4 	 #18  	 打劫！我有枪的
    #       225ECF82 	 #101 	 干杯
    #       247121AF 	 #124 	 晕
    #       270D5C71 	 #35  	 不是吧？
    #       2DF12D10 	 #127 	 可爱玲珑兔 <- can not find this one
    #       30615DBC 	 #20  	 我是GG
    #       3694C64F 	 #21  	 送你一个吻
    #       383F3815 	 #121 	 演讲
    #       396C4E03 	 #80  	 鄙视你
    #       3B3D19C0 	 #12  	 到点了
    #       403105F2 	 #2   	 顽皮的笑
    #       445A8BA0 	 #122 	 变猪猪
    #       44B657A9 	 #46  	 快一点，等不及啦！
    #       4806AE3B 	 #108 	 无聊
    #       488EBBD6 	 #107 	 拜拜
    #       4D3D1188 	 #0   	 微笑
    #       4E20C2E6 	 #45  	 好惊讶！怎么会这样呢？
    #       4FAD347C 	 #23  	 送你红玫瑰
    #       4FF6E07A 	 #105 	 快来人啊
    #       50BF3749 	 #120 	 囧
    #       522BC68F 	 #48  	 我很鬼，是小妖怪。
    #       525FCCF9 	 #15  	 555你欺侮我
    #       572F2A4D 	 #8   	 心碎了
    #       57648A83 	 #128 	 一起卖萌
    #       5887677B 	 #112 	 见鬼啦
    #       58C9FAB0 	 #62  	 看我表演！
    #       58FAA400 	 #34  	 我困了
    #       590CAA9B 	 #44  	 得意忘形的笑，得意的笑……
    #       5BA9CF5E 	 #29  	 猪头～
    #       64A8BD13 	 #125 	 浮云
    #       65D48DBF 	 #118 	 拍马屁
    #       66D0E07C 	 #41  	 好高兴好高兴！
    #       6E7A8E8F 	 #1   	 开口笑
    #       707ABF50 	 #61  	 真的吗？太高兴啦！
    #       7229A70C 	 #104 	 是他干的
    #       73F3BF9D 	 #39  	 好热呀，扇扇风！
    #       743AF90F 	 #24  	 不会吧
    #       74E0F5FA 	 #27  	 菜鸟（表示否定）
    #       79C2D9F2 	 #126 	 神马
    #       7A9F28C7 	 #110 	 闪啦
    #       7EEB3422 	 #89  	 流口水
    #       7F869E1E 	 #96  	 来亲一个
    #       853F3BC9 	 #25  	 鬼脸
    #       85AC8CCB 	 #60  	 我生气了……哈哈，傻瓜，骗你的。
    #       87621B9F 	 #78  	 无可奈何
    #       888536BF 	 #31  	 请你喝可乐
    #       8E0063E2 	 #28  	 扮酷
    #       8F20BE2E 	 #90  	 超人来啦
    #       91EAD158 	 #119 	 寒
    #       978B9123 	 #57  	 打电话给我。
    #       978F8F8A 	 #47  	 哇哈哈～
    #       99AFED62 	 #99  	 流鼻血
    #       9A8BFB91 	 #42  	 我会唱歌，吹笛子。
    #       9EEC6DE4 	 #13  	 休息一下
    #       A1E13E27 	 #9   	 让你看我的真心
    #       A1E7B566 	 #91  	 不理你了
    #       A5D718B1 	 #6   	 我们是哥们，干杯！
    #       A8A9B15D 	 #49  	 恩，很舒服。
    #       A8BC861D 	 #103 	 给你写信
    #       AA7B3B42 	 #63  	 哇，真的被你吓到了！
    #       AAFBD630 	 #114 	 举杠铃
    #       ACA32B8F 	 #116 	 摸摸
    #       ACE9C474 	 #36  	 人家暗恋你好久啦……
    #       AD9E8BAD 	 #16  	 我是小恶魔
    #       ADE1576E 	 #82  	 我回来了
    #       B06B70C0 	 #81  	 吃饭咯
    #       B2F4A198 	 #10  	 生日快乐
    #       B5786848 	 #87  	 不要啊
    #       B7E060C1 	 #111 	 兴高采烈
    #       BE3150EE 	 #37  	 送你一朵小红花！
    #       BEDE7D41 	 #32  	 快来啊
    #       C2A7A47D 	 #88  	 升级咯
    #       C699AB3E 	 #53  	 翻筋斗
    #       C8AA7848 	 #5   	 蝙蝠侠，呵呵
    #       C8BBEEA3 	 #100 	 强
    #       CA47B474 	 #43  	 真激动呀！
    #       CCD6B7E8 	 #40  	 休息一下，擦擦汗。
    #       CD8F0AD6 	 #56  	 暗自开心，洋洋得意。
    #       CDC95381 	 #79  	 我是小护士
    #       D086F684 	 #75  	 给你一锤子
    #       D3C23894 	 #3   	 晕倒
    #       D5C14B62 	 #102 	 送花给你！
    #       D6252D94 	 #123 	 乐极生悲
    #       D6436048 	 #26  	 你真了不起（表示肯定）
    #       D753949E 	 #129 	 拍死
    #       DC9C1E87 	 #86  	 开大餐咯
    #       DF1F56AC 	 #106 	 火烧pp啦
    #       E0C6F0D3 	 #7   	 小心我咬你
    #       E4994B6A 	 #115 	 仰卧起坐
    #       E53DE56A 	 #50  	 鼓掌。
    #       E5FF2DE2 	 #77  	 睡觉觉
    #       E88B5354 	 #51  	 大声叫好！
    #       E8E08FA9 	 #30  	 嘿！我想到了
    #       E9A1E271 	 #17  	 人家脸红了
    #       ED5B5996 	 #117 	 悲剧
    #       EDA67286 	 #84  	 好厉害哦
    #       EDD63AB1 	 #4   	 我生气了
    #       EDEBCFCF 	 #11  	 交个朋友吧
    #       EF498C25 	 #93  	 加油啊加油
    #       F06B6B9E 	 #33  	 送你小礼物
    #       F2FBDA6E 	 #64  	 兴高采烈，非常开心
    #       F45DCF6A 	 #98  	 我们好恩爱
    #       F5509B1C 	 #95  	 抽根烟休息一下
    #       F95512DC 	 #94  	 有信来了
    #       FB472367 	 #83  	 流汗
    #       FC4215EC 	 #74  	 呕吐啊
    #       FCCAA9B5 	 #76  	 给我闭嘴
    #       FCD58523 	 #55  	 快点告诉我。
    #       FD438646 	 #22  	 有空call me
    #
    # 2. extract .was files of emojis from the .wdf resource
    #    uses ../bin/梦幻西游素材提取工具、染色工具、资源导出工具包.zip/wdf压缩包内所有图片资源快速查看、导出工具
    #
    # 3. convert .was files to .png files
    #    uses ../bin/梦幻西游素材提取工具、染色工具、资源导出工具包.zip/was图片资源超智能管理、导出透明PNG工具
    #    this .was tool also has #0~#99 emojis, actually we only need to extract emojis from .wdf for all emojis with index >= 100
    #
    # 4. use this script to convert to emoji format for mir2x
    #    use abstract emoji index, because we may create NPC chat text with emojis, but later we can add new emojis
    #    to avoid bad emoji, for each emoji if assigned an index, we never replace/change it

    if len(sys.argv) != 4:
        raise ValueError("usage: python3 png2emoji <repo-dir> <emoji-set> <output-dir>")
    convert_emoji_repo(sys.argv[1], int(sys.argv[2]), sys.argv[3])


if __name__ == "__main__":
    main()
