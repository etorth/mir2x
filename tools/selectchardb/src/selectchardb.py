import os
import sys
import glob
import shutil

# job: 0 战士
#      1 道士
#      2 法师

# gender: 0 女
#         1 男

# motion: 0 stand, inactive
#         1 on select
#         2 stand, active
#         3 on deselect
#         4 on create char in ProcessCreateChar, not used here

def get_start_index(job, gender):
    if job == 0 and gender == 0: return  500
    if job == 1 and gender == 0: return 1700
    if job == 2 and gender == 0: return 1100
    if job == 0 and gender == 1: return  200
    if job == 1 and gender == 1: return 1400
    if job == 2 and gender == 1: return  800
    raise ValueError(job, gender)


def get_old_index(job, gender, motion, frame):
    return get_start_index(job, gender) + motion * 60 + frame

def db_file_exists(db_path, index):
    flist = glob.glob('%s/TMP??????_%08X_??????????_M.PNG' % (db_path, index))
    if len(flist) == 0:
        return None
    elif len(flist) == 1:
        return flist[0]
    else:
        raise ValueError(index, flist)


def get_new_index(job, gender, motion, frame):
    return (job << 10) | (gender << 9) | (motion << 5) | frame


def copy_2_new_index(src, out_path, new_index):
    offstr = src[-16:-6]
    shutil.copy2(src, '%s/%08X%s.PNG' % (out_path, new_index, offstr))


def selectchardb(db_path, out_path):
    if not os.path.exists(db_path):
        raise ValueError("input path doesn't exists: %s" % db_path)

    if not os.path.exists(db_path):
        raise ValueError("output path doesn't exists: %s" % out_path)

    for job in [0, 1, 2]:
        for gender in [0, 1]:
            for motion in range(0, 5):
                for frame in range(0, 20):
                    old_index = get_old_index(job, gender, motion, frame)
                    new_index = get_new_index(job, gender, motion, frame)

                    mask_shadow = 1 << 14
                    mask_magic  = 1 << 13

                    body_file = db_file_exists(db_path, old_index)
                    if body_file:
                        copy_2_new_index(body_file, out_path, new_index)

                        shadow_file = db_file_exists(db_path, old_index + 20)
                        if shadow_file:
                            copy_2_new_index(shadow_file, out_path, new_index | mask_shadow)

                        magic_file = db_file_exists(db_path, old_index + 40)
                        if magic_file:
                            copy_2_new_index(magic_file, out_path, new_index | mask_magic)


def main():
    # convert raw index in wil to PNGTexOffDB:
    # but still need to add alpha channel to magic files
    #
    #     TMP001968_000007B0_0100300111_M.PNG
    #     TMP001969_000007B1_0100300111_M.PNG
    #     TMP001970_000007B2_0100330111_M.PNG
    #
    # new index see code: mir2x/client/src/processselectchar.cpp: drawChar()

    if len(sys.argv) != 3:
        raise ValueError("usage: python3 selectchardb.py <db_path> <out_path>")
    selectchardb(sys.argv[1], sys.argv[2])


if __name__ == "__main__":
    main()
