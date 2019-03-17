import opensimplex
import colorama
from PIL import Image

def OctaveNoiseMixin(detail):
    class OctaveNoiseMixin_impl:
        DETAIL = detail
        OCTAVES = tuple(
            (2**x, 2**-x) for x in range(0, detail)
        )
        
        def octave2d(self, x, y):
            return sum(
                    (self.noise2d(x*a, y*a) + 1) * (b / 2) for a, b in self.OCTAVES
            ) / 2.0

    OctaveNoiseMixin_impl.__name__ += "_" + str(detail)
    return OctaveNoiseMixin_impl


class OctaveNoise4(opensimplex.OpenSimplex, OctaveNoiseMixin(4)):
    pass

noise = OctaveNoise4(1)

# marching squares

# really just fancy for estimating contours with samples -- should allow me to do ascii-ification of thresholds of noisemaps:

# format:
#     - abcd
#       0011:
# 
#       a  b
# 
#       c  d

MARCHING = {
    0b0000: ' ',
    0b0001: ' ',
    0b0010: ' ',
    0b0100: ' ',
    0b1000: ' ',
    0b0011: '_',
    0b1100: '‾',
    0b0101: '|',
    0b1010: '|',
    0b1110: '/',
    0b1101: '\\',
    0b0111: '/',
    0b1011: '\\',
    0b0110: '/',
    0b1001: '\\'
}  # you can only do so much with ascii

ROWS = 77
COLS = 306

ADVANCE_X = 1/2.5
ADVANCE_Y = 1

THRESH = 0.5

SCALE = 1/18.0

samples = [[False] * (ROWS + 1) for _ in range(COLS + 1)]
solids = [[False] * (ROWS + 1) for _ in range(COLS + 1)]

for x in range(COLS+1):
    for y in range(ROWS+1):
        value = noise.octave2d(x * SCALE * ADVANCE_X, y * SCALE * ADVANCE_Y)
        samples[x][y] = value > THRESH
        solids[x][y] =  value > 0.6

for y in range(ROWS):
    for x in range(COLS):
        print("#" if samples[x][y] else " ", end="")
    print()

for y in range(ROWS):
    for x in range(COLS):
        bitmask = (1 << 3) * samples[x][y] + \
                  (1 << 2) * samples[(x+1)][y] + \
                  (1 << 1) * samples[x][(y+1)] + \
                  (1 << 0) * samples[(x+1)][(y+1)]
        if bitmask != 0b1111:
            print(colorama.Back.RESET + colorama.Fore.LIGHTWHITE_EX + MARCHING[bitmask], end='')
        else:
            if solids[x][y] and solids[x+1][y] and solids[x][y+1] and solids[x+1][y+1]:
                print(colorama.Back.LIGHTBLACK_EX + colorama.Fore.BLUE + '#' + colorama.Fore.RESET, end='')
            else:
                print(colorama.Back.LIGHTBLACK_EX + ' ', end='')
    print()
