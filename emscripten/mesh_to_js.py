import io,sys


def main(argv):
    inf = open(argv[1])
    vtx = []
    norm = []
    meshes = {}
    for lines in inf:
        line = lines.split()
        if line[0] == 'v':
            assert len(line) == 7
            vtx.extend(line[1:4])
            norm.extend(line[4:7])
        elif line[0][0] == '*':
            meshno = line[0][1:]
            cmesh = []
            meshes[meshno] = cmesh
        elif len(line) == 4:
            cmesh.append(line[0])
            cmesh.append(line[2])
            cmesh.append(line[1])
            cmesh.append(line[0])
            cmesh.append(line[3])
            cmesh.append(line[2])
        elif len(line) != 0:
            raise Exception('unknown ' + line)
            
    outf = open(argv[2], 'w')
    outf.write('var unimesh={\n  vtx: new Float32Array([' + ','.join(vtx) + ']),\n')
    outf.write('  norm: new Float32Array([' + ','.join(norm) + ']),\n')
    for k,m in meshes.iteritems():
        outf.write('  0x' + k + ': new Int16Array([' + ','.join(m) + ']),\n')
    outf.write('}')

main(sys.argv)
# mesh_to_js.py ..\unified_meshes_all.txt unified_mesh.js