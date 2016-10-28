var uni_pcs = [
    {
        famName: "Happy Cube",
        drawType: "color",
        onResetSel: 1,
        menuIndex: 2,
        p: [
         { name: "HC-Milano",    sig:"ab6cb492a492b6952a", cb:"#4981FF"},  // blue
         { name: "HC-NewYork",   sig:"9a9084149bacb69524", cb:"#2BDD38"}, // green
         { name: "HC-Tokyo",     sig:"845285ac55e9b6b22a", cb:"#FFFF00"}, // yellow
         { name: "HC-Amsterdam", sig:"ca549ba88bd4a82b5b", cb:"#FF8C00"}, // orange
         { name: "HC-Paris",     sig:"6859a41da46c55ad2a", cb:"#FF2B30"}, // red
         { name: "HC-Brussels",  sig:"6868bb6eda2a496326", cb:"#940088"} // purple
        ]
    },
    {
        famName: "Black Profi Cube",
        drawType: "mix",
        lines: "black",
        menuIndex: 3,
        p: [
         { name: "PC-Confusius", sig:"64935a9754ed566a25", cb:"#2020FF", cf:"#151515"}, // blue
         { name: "PC-DaVinci",   sig:"5bd5a4684a11496a2b", cb:"#F4EC14", cf:"#151515"}, // yellow
         { name: "PC-MarcoPolo", sig:"4469b56aa5ac56ad24", cb:"#EC0611", cf:"#151515"}, // red
         { name: "PC-Rubens",    sig:"6453442b6469b56a2b", cb:"#820064", cf:"#151515"}, // purple
         { name: "PC-Watt",      sig:"456d73118ba84a9525", cb:"#646464", cf:"#151515"}, // gray
         { name: "PC-Newton",    sig:"4610a5549bd2b66a54", cb:"#171717", drawType: "color"} // black
        ]
    },
    {
        famName: "Marble Cube",
        drawType: "marble",
        menuIndex: 5,
        p: [                           
        { name: "MC-M.L. King",   sig:"94a8c46eea54d66a4d", cb:"#3778FF", cf:"#8BD9FF" }, //blue
        { name: "MC-O. Khayyam",  sig:"9b105aad6b6b496a2b", cb:"#44D115", cf:"#A7FF88"}, // green
        { name: "MC-Marie Curie", sig:"846c532b9a0da78a2b", cb:"#FFF499", cf:"#FFE708" }, // yellow
        { name: "MC-B. Fuller",   sig:"aa54ba746a1335ad2a", cb:"#E01361", cf:"#FA83A2"}, // pink
        { name: "MC-M. Gandhi",   sig:"6b1165118395ac8d39", cb:"#FF7B7B", cf:"#E40303"}, //red
        { name: "MC-A. Einstein", sig:"86f16a91195396dd62", cb:"#7F0AD9", cf:"#CA7CFC"} // purple
        ]
    }, //sig:"", cb:"", cf:""
    {
        famName: "Little Genius",
        drawType: "tex",
        menuIndex: 0,
        p: [
        { name: "LG-Nature",    sig:"64558b2e8aaa4aad2a", cf:"#2BDD38", cb:"#419ae8", img:"genius/nature.svg"},
        { name: "LG-Fruits",    sig:"84d0a4524b6d49925a", cf:"#419AE8", cb:"#48de44", img:"genius/fruits.svg"}, 
        { name: "LG-Animals",   sig:"64934415446d55522b", cf:"#F8A632", cb:"#ffff00", img:"genius/animals.svg"}, 
        { name: "LG-Emotions",  sig:"a46844135a53a9ad5a", cf:"#FFFF00", cb:"#ff9b12", img:"genius/emoticons.svg"}, 
        { name: "LG-Transport", sig:"84109b565b6d555555", cf:"#BC00B1", cb:"#ff3d3d", img:"genius/transport.svg"}, 
        { name: "LG-Symbols",   sig:"94524a295a5155555b", cf:"#FF3D3D", cb:"#bc00b1", img:"genius/symbols.svg"} 
        ]
    },   
    {
        famName: "Color Profi Cube",
        drawType: "mix",
        menuIndex: 4,
        p: [
         { name: "CPC-Confusius", sig:"64935a9754ed566a25", cb:"#6FB9D2", cf:"#0F929F"}, // blue
         { name: "CPC-DaVinci",   sig:"5bd5a4684a11496a2b", cb:"#5EFF41", cf:"#00D8AD"}, // yellow
         { name: "CPC-MarcoPolo", sig:"4469b56aa5ac56ad24", cb:"#FBFB07", cf:"#00D5CB"}, // red
         { name: "CPC-Rubens",    sig:"6453442b6469b56a2b", cb:"#FF7200", cf:"#FFA300"}, // purple
         { name: "CPC-Watt",      sig:"456d73118ba84a9525", cb:"#FF2A00", cf:"#F60000"}, // gray
         { name: "CPC-Newton",    sig:"4610a5549bd2b66a54", cb:"#A71499", cf:"#FF2538"} // black
        ]
    },
    {
        famName: "Micro Cube",
        drawType: "color",
        menuIndex: 7,
        p: [
         { name: "UC-Barcelona",  sig:"a410b5ac5bebb6ad24", cb:"#4981FF"},  // blue
         { name: "UC-Bern",       sig:"6b6b9494846c556a25", cb:"#2BDD38"}, // green
         { name: "UC-Birmingham", sig:"66716455146da59254", cb:"#FFFF00"}, // yellow
         { name: "UC-Glasgow",    sig:"8b68abaf5551555255", cb:"#FF8C00"}, // orange
         { name: "UC-Manchester", sig:"9a50ab2ca528b5ad5a", cb:"#FF2B30"}, // red
         { name: "UC-Oxford",     sig:"9bd0742f8a0c4d8d5a", cb:"#940088"} // purple
        ]
    },    
    {
        famName: "Smart Cube", // was Baubox
        drawType: "color",
        menuIndex: 6,
        p: [
         { name: "SC-Blue",   sig:"44559b2c9ba84aad2a", cb:"#4981FF"},  // blue
         { name: "SC-Green",  sig:"44559b2c9ba84aad2a", cb:"#2BDD38"}, // green
         { name: "SC-Yellow", sig:"44559b2c9ba84aad2a", cb:"#FFFF00"}, // yellow
         { name: "SC-Orange", sig:"44559b2c9ba84aad2a", cb:"#FF8C00"}, // orange
         { name: "SC-Red",    sig:"44559b2c9ba84aad2a", cb:"#FF2B30"}, // red
         { name: "SC-Purple", sig:"44559b2c9ba84aad2a", cb:"#940088"} // purple
        ]
    },    
    {
        famName: "New Little Genius", // same pieces, different drawings
        drawType: "tex",
        menuIndex: 1,
        p: [
        { name: "NLG-Fruit",    sig:"64558b2e8aaa4aad2a", cb:"#008fc5", img:"genius/nlg_fruit.svg"},
        { name: "NLG-Animals",  sig:"84d0a4524b6d49925a", cb:"#c1d82f", img:"genius/nlg_animals.svg"}, 
        { name: "NLG-Vehicles", sig:"64934415446d55522b", cb:"#ffd200", img:"genius/nlg_vehicles.svg"}, 
        { name: "NLG-Sports",   sig:"a46844135a53a9ad5a", cb:"#f78e1e", img:"genius/nlg_sports.svg"}, 
        { name: "NLG-Weather",  sig:"84109b565b6d555555", cb:"#ee3124", img:"genius/nlg_weather.svg"}, 
        { name: "NLG-Nature",   sig:"94524a295a5155555b", cb:"#7f3f98", img:"genius/nlg_nature.svg"} 
        ]
    }, 
        
    
]
