import sys
import os

def main(argv):
    if len(argv) < 3:
        print('usage:\n  python makeString.py input output')
        sys.exit(1)
    filename = argv[1]
    name = filename.split('\\')[-1].replace(".", "_")
    
    fout = open(argv[2], 'w')
    fout.write('//generated code from %s\n\nconst char *code_%s = " \\\n'%(filename, name))
    
    fin = open(filename, 'r')
    
    for line in fin:
        fout.write('  %s \\n\\\n'%line.rstrip('\r\n'))
    fout.write('  ";\n')

if __name__ == '__main__':
    main(sys.argv) 
    

    