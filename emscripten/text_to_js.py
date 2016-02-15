import sys,os

def main(argv):
    text = open(argv[1]).read()
    out = "var " + argv[3] + " = '"
    out += text.replace('\n', "\\n\\\n")
    out += "'"
    open(argv[2], 'w').write(out)

if __name__ == "__main__":
    sys.exit(main(sys.argv))
    
    
# text_to_js.py ..\stdpcs.xml stdpcs.js stdpcs_text
# text_to_js.py one_marble_cube.txt one_marble_cube.js one_marble_cube_text 
# text_to_js.py one_piece_cube.txt one_piece_cube.js one_piece_cube_text