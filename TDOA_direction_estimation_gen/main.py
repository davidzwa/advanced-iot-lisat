import numpy as np

if __name__ == '__main__':
    num_mics = int(input("number of microphones: "))

    mic_loc = np.zeros((num_mics, 2))

    for i in range(num_mics):
        mic_loc[i, 0] = input("x{} = ".format(i))
        mic_loc[i, 1] = input("y{} = ".format(i))

    print("Robot has the following relative microphone locations")
    print(mic_loc)

    mic_diff_loc = np.zeros((num_mics - 1, 2))
    for i in range(num_mics - 1):
        mic_diff_loc[i, 0] = mic_loc[i + 1, 0] - mic_loc[0, 0]
        mic_diff_loc[i, 1] = mic_loc[i + 1, 1] - mic_loc[0, 1]

    inv_mic_dif_loc = np.linalg.pinv(mic_diff_loc)

    print("(pseudo) inverse of the difference in microphone locations matrix")
    print(inv_mic_dif_loc)

    # writing C code

    file = open(r"TDOA_direction_code.txt", 'w')

    line = 'C++ code for TDOA direction estimation\n' \
           'requires: math.h\n' \
           '\n' \
           'Function declaration: \n' \
           'void TDOA_direction_estimation(int[{}], float[2]);\n\n'.format(num_mics-1)

    line = line + 'Function:\n' \
                  'void TDOA_direction_estimation(int TDOA[{}], float dir[2])\n'.format(num_mics-1)

    line = line + '{\n' \
                  '\t// TDOA = array of time differences\n' \
                  '\t// dir = direction array (estimate array will be saved there)\n' \
                  '\n'

    line = line + '\tconst float inverse[2][{}] = '.format(num_mics - 1) + '{'

    for j in range(2):
        line = line + '{'
        for i in range(num_mics - 1):
            line = line + ' {},'.format(inv_mic_dif_loc[j][i])
        line = line[:-1] + '},'

    line = line[:-1] + '};\n'

    line = line + '\tfloat mag;\n' \
                  '\n' \
                  '\tdir[0] = dir[1] = 0;\n' \
                  '\n' \
                  '\t// matrix multiplication\n' \
                  '\tfor (int i = 0; i < {}; i ++)\n'.format(num_mics-1)

    line = line + '\t\t{\n' \
                  '\t\t\tdir[0] += inverse[0][i] * TDOA[i];\n' \
                  '\t\t\tdir[1] += inverse[1][i] * TDOA[i];\n' \
                  '\t\t}\n' \
                  '\n' \
                  '\t// normalize vector\n' \
                  '\tmag = sqrt(dir[0] * dir[0] + dir[1] * dir[1]);\n' \
                  '\tdir[0] = dir[0] / mag;\n' \
                  '\tdir[1] = dir[1] / mag;\n' \
                  '}'

    file.write(line)

    file.close()

    input("enter to close this window")

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
