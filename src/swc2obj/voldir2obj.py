# trace generated using paraview version 5.8.1
#
# To ensure correct image size when batch processing, please search
# for and uncomment the line `# renderView*.ViewSize = [*,*]`

#### import the simple module from the paraview

import sys
import getopt
import os
from paraview.simple import *

if __name__ == "__main__":
    argv = sys.argv[1:]
    inputdir = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(argv, "hi:o:", ["idir=", "ofile="])
    except getopt.GetoptError:
        print('test.py -i <inputdir> -o <outputfile>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('test.py -i <inputdir> -o <outputfile>')
            sys.exit()
        elif opt in ("-i", "--idir"):
            inputdir = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg
    print('输入的文件夹：', inputdir)
    print('输出的文件为：', outputfile)

    filenames = []

    for filename in os.listdir(inputdir):
        if filename[0]!='.' and filename[-1]!='w':
            filenames.append(inputdir+'/'+filename)
            print('input file is '+filename)

    paraview.simple._DisableFirstRenderCameraReset()

    allcontour =  []
    for filename in filenames:
        tmhd = MetaFileSeriesReader(FileNames=[filename])
        materialLibrary1 = GetMaterialLibrary()

        contour1 = Contour(Input=tmhd)
        contour1.ContourBy = ['POINTS', 'MetaImage']
        contour1.Isosurfaces = [0.1]
        contour1.PointMergeMethod = 'Uniform Binning'
        allcontour.append(contour1)
    appendGeometry1 = AppendGeometry(Input=allcontour)

    # save data
    SaveData(outputfile, proxy=appendGeometry1, ChooseArraysToWrite=1,
             PointDataArrays=['MetaImage', 'Normals'])

