import trimesh as tm
import sys
import getopt
import os

if __name__ == "__main__":

    argv = sys.argv[1:]
    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(argv, "hi:o:", ["ifile=", "ofile="])
    except getopt.GetoptError:
        print('test.py -i <inputfile> -o <outputfile>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('test.py -i <inputfile> -o <outputfile>')
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg
    print('输入的文件夹：', inputfile)
    print('输出的文件为：', outputfile)


    f = open(inputfile,'r+')
    obj = tm.exchange.obj.load_obj(f)
    obj1 = tm.Trimesh(vertices=obj['vertices'],faces=obj['faces'],vertex_normals=obj['vertex_normals'])
    obj2 = tm.smoothing.filter_taubin(obj1,lamb=0.5, nu=-0.1, iterations=10)
    objz = tm.exchange.obj.export_obj(obj2,include_normals=True)
    with open(outputfile,'w+') as f:
        f.write(objz)