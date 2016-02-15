var stdpcs_text = '<?xml version="1.0" encoding="utf-8"?>\n\
<pieces>\n\
	<textures>\n\
		<texture filename=":/black64.png"/>\n\
		<texture filename=":/marbleg2_2.png"/>\n\
		<texture filename=":/littlegenious128_2.png" in3d="true"/>\n\
		<texture filename=":/new_genious128.png" in3d="true"/>\n\
	</textures>\n\
	<outlines>\n\
		<family name="Happy Cube" index="2" onResetSetCount="1" size="8mm" icon="images/sampleHappyCube.png">\n\
			<group name="HC-Milano"> <!-- blue -->\n\
				<fill type="COLOR" k="0" r="73" g="129" b="255"/>\n\
				<piece>1 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 1 0 1 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  1 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 1 0 1 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 0 1 0 0</piece>\n\
			</group>\n\
			<group name="HC-NewYork"> <!-- green -->\n\
				<fill type="COLOR" k="0" r="43" g="221" b="56"/>\n\
				<piece>1 1 0 1 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 1</piece>\n\
			</group>\n\
			<group name="HC-Tokyo"> <!-- yellow -->\n\
				<fill type="COLOR" k="0" r="255" g="255" b="0"/>\n\
				<piece>0 0 1 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece>0 0 0 1 1  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 1 0 1 0</piece>\n\
				<piece>1 1 0 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 1</piece>\n\
				<piece>0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 1</piece>\n\
			</group>\n\
			<group name="HC-Amsterdam"> <!-- orange -->\n\
				<fill type="COLOR" k="0" r="255" g="140" n="0"/>\n\
				<piece>0 1 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 1</piece>\n\
				<piece>0 1 0 1 0  0 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  1 1 1 1 1  0 1 1 1 0  0 0 1 0 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 1 0 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
			</group>\n\
			<group name="HC-Paris"> <!-- red -->\n\
				<fill type="COLOR" k="0" r="255" g="43" b="48"/>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 1 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 1 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 0 1 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 1 0 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>1 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 1 0</piece>\n\
			</group>\n\
			<group name="HC-Brussels"> <!-- purple -->\n\
				<fill type="COLOR" k="0" r="148" g="0" b="136"/>\n\
				<piece>1 1 0 1 1  0 1 1 1 1  1 1 1 1 0  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  1 1 1 1 0  1 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece>0 1 0 1 1  0 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece>0 1 1 0 1  0 1 1 1 1  1 1 1 1 1  1 1 1 1 0  1 0 1 0 0</piece>\n\
				<piece>0 0 0 1 1  1 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
			</group>\n\
		</family>\n\
		<family name="Black Profi Cube" index="3" onResetSetCount="0" size="8mm" icon="images/sampleBlackProfi.png">\n\
			<group name="PC-Confusius">\n\
				<fill type="TEXTURE_BLEND" texind="0" k="2" r="0" g="0" b="202" exR="20" exG="20" exB="20"/>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece>0 0 1 0 1  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  1 1 0 1 1</piece>\n\
				<piece>0 1 0 1 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  1 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 1</piece>\n\
			</group>\n\
			<group name="PC-DaVinci">\n\
				<fill type="TEXTURE_BLEND" texind="0" k="2" r="244" g="236" b="20" exR="20" exG="20" exB="20"/>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 1 0 1 0</piece>\n\
				<piece>0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  1 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 1 0 1 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
			</group>\n\
			<group name="PC-MarcoPolo">\n\
				<fill type="TEXTURE_BLEND" texind="0" k="2" r="236" g="6" b="17" exR="20" exG="20" exB="20"/>\n\
				<piece>0 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece>1 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  1 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
			</group>\n\
			<group name="PC-Rubens">\n\
				<fill type="TEXTURE_BLEND" texind="0" k="2" r="130" g="0" b="100" exR="20" exG="20" exB="20"/>\n\
				<piece>0 1 0 1 1  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>1 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
			</group>\n\
			<group name="PC-Watt">\n\
				<fill type="TEXTURE_BLEND" texind="0" k="2" r="100" g="100" b="100" exR="20" exG="20" exB="20"/>\n\
				<piece>1 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 1 0 1 0  1 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 0  1 1 1 1 1  1 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 1</piece>\n\
			</group>\n\
			<group name="PC-Newton">\n\
				<fill type="COLOR" k="2" r="40" g="40" b="40"/>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 1 0 0</piece>\n\
				<piece>1 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 1 0 1 1</piece>\n\
				<piece>0 0 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  1 1 1 1 0  1 1 1 1 1  0 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 0  1 1 1 1 1  1 1 0 1 0</piece>\n\
			</group>\n\
		</family>\n\
		<family name="Marble Cube" index="5" onResetSetCount="0" size="8mm" icon="images/sampleMarble.png">\n\
			<group name="MC-Martin L. King">\n\
				<fill type="TEXTURE_MARBLE" texind="1" k="0" r="55" g="120" b="255" exR="139" exG="217" exB="255"/>\n\
				<piece>0 0 1 0 1  0 1 1 1 1  0 1 1 1 0  1 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  1 1 1 1 0  0 0 0 1 0</piece>\n\
				<piece>0 1 0 1 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 1 0 1 0</piece>\n\
				<piece>0 1 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 1 0 0</piece>\n\
				<piece>1 1 0 1 1  1 1 1 1 0  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
			</group>\n\
			<group name="MC-Omar Khayyam">\n\
				<fill type="TEXTURE_MARBLE" texind="1" k="0" r="68" g="209" b="21" exR="167" exG="255" exB="136"/>\n\
				<piece>1 1 0 1 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 1 0 1 0</piece>\n\
				<piece>0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece>0 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 0 1 0 0</piece>\n\
			</group>\n\
			<group name="MC-Marie Curie">\n\
				<fill type="TEXTURE_MARBLE" texind="1" k="0" r="255" g="244" b="153" exR="255" exG="231" exB="8"/>\n\
				<piece>1 1 0 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>1 1 0 0 0  1 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 1 1 0 0  0 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece>0 0 0 1 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 1 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 1 0 0</piece>\n\
			</group>\n\
			<group name="MC-Buckminster Fuller">\n\
				<fill type="TEXTURE_MARBLE" texind="1" k="0" r="244" g="38" b="116" exR="252" exG="179" exB="198"/>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 1 0 1 0</piece>\n\
				<piece>0 0 0 1 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  1 1 1 1 0  1 0 1 0 0</piece>\n\
				<piece>0 0 1 0 1  0 1 1 1 1  1 1 1 1 0  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 1 0 1 1  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  1 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
			</group>\n\
			<group name="MC-Mahatma Gandhi">\n\
				<fill type="TEXTURE_MARBLE" texind="1" k="0" r="255" g="144" b="122" exR="255" exG="12" exB="0"/>\n\
				<piece>0 1 0 1 1  0 1 1 1 1  1 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 1 0 1 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 0  0 0 1 1 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 0 1 1 1</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  0 1 1 1 0  1 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  1 1 1 1 1  1 1 1 1 0  1 0 1 0 0</piece>\n\
				<piece>1 1 0 1 0  1 1 1 1 0  0 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
			</group>\n\
			<group name="MC-Albert Einstein">\n\
				<fill type="TEXTURE_MARBLE" texind="1" k="0" r="142" g="10" b="217" exR="222" exG="157" exB="255"/>\n\
				<piece>0 0 0 1 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 1 1 0 0  0 1 1 1 0  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 1 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 0 1 1 1</piece>\n\
				<piece>0 1 1 0 0  0 1 1 1 0  1 1 1 1 1  1 1 1 1 0  0 0 0 1 1</piece>\n\
				<piece>0 0 1 1 1  1 1 1 1 1  1 1 1 1 0  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>1 0 0 1 1  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
			</group>\n\
		</family>\n\
		<family name="Little Genius" index="0" onResetSetCount="0" size="8mm" icon="images/sampleLittleGenius.png">\n\
			<group name="LG-Nature">\n\
				<fill type="TEXTURE_INDIVIDUAL_HALF" texind="2" k="0" r="43" g="221" b="56" />\n\
				<piece x1="32"  y1="32">0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 1</piece>\n\
				<piece x1="160" y1="32">0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 1</piece>\n\
				<piece x1="288" y1="32">0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece x1="416" y1="32">0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 0</piece>\n\
				<piece x1="544" y1="32">1 1 0 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece x1="672" y1="32">0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 1</piece>\n\
			</group>\n\
			<group name="LG-Fruits">\n\
				<fill type="TEXTURE_INDIVIDUAL_HALF" texind="2" k="0" r="65" g="154" b="232" />\n\
				<piece x1="32"  y1="192">0 0 1 0 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  1 1 0 1 0</piece>\n\
				<piece x1="160" y1="192">1 1 0 1 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece x1="288" y1="192">0 0 1 0 1  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 1 0 1 0</piece>\n\
				<piece x1="416" y1="192">0 0 1 0 1  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  1 1 0 1 1</piece>\n\
				<piece x1="544" y1="192">1 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece x1="672" y1="192">1 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
			</group>\n\
			<group name="LG-Animals">\n\
				<fill type="TEXTURE_INDIVIDUAL_HALF" texind="2" k="0" r="248" g="166" b="50" />\n\
				<piece x1="32"  y1="352">1 1 0 1 1  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece x1="160" y1="352">0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece x1="288" y1="352">1 1 0 1 1  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 0 1 0 0</piece>\n\
				<piece x1="416" y1="352">0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece x1="544" y1="352">1 1 0 1 1  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece x1="672" y1="352">1 1 0 1 1  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 0 1 0 0</piece>\n\
			</group>\n\
			<group name="LG-Emotions">\n\
				<fill type="TEXTURE_INDIVIDUAL_HALF" texind="2" k="0" r="255" g="255" b="0" />\n\
				<piece x1="32"  y1="512">0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 0 1 0 0</piece>\n\
				<piece x1="160" y1="512">0 1 0 1 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 1 0 1 1</piece>\n\
				<piece x1="288" y1="512">0 0 1 0 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 0 1 0 1</piece>\n\
				<piece x1="416" y1="512">0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 1</piece>\n\
				<piece x1="544" y1="512">0 0 1 0 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 0 1 0 1</piece>\n\
				<piece x1="672" y1="512">1 1 0 1 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 1 0 1 0</piece>\n\
			</group>\n\
			<group name="LG-Transport">\n\
				<fill type="TEXTURE_INDIVIDUAL_HALF" texind="2" k="0" r="188" g="0" b="177" />\n\
				<piece x1="32"  y1="672">1 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 1 0 1 1</piece>\n\
				<piece x1="160" y1="672">0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece x1="288" y1="672">0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 0 1 0 0</piece>\n\
				<piece x1="416" y1="672">0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 0 1 0 0</piece>\n\
				<piece x1="544" y1="672">1 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 1 0 1 1</piece>\n\
				<piece x1="672" y1="672">0 0 1 0 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 1 0 1 0</piece>\n\
			</group>\n\
			<group name="LG-Symbols">\n\
				<fill type="TEXTURE_INDIVIDUAL_HALF" texind="2" k="0" r="255" g="61" b="61" />\n\
				<piece x1="32"  y1="832">0 1 0 1 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece x1="160" y1="832">1 1 0 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 0 1 0 0</piece>\n\
				<piece x1="288" y1="832">1 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 1 0 1 0</piece>\n\
				<piece x1="416" y1="832">0 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 1 0 1 0</piece>\n\
				<piece x1="544" y1="832">1 1 0 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 0 1 0 0</piece>\n\
				<piece x1="672" y1="832">1 1 0 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 0 1 0 0</piece>\n\
			</group>\n\
		</family>\n\
		<family name ="Color Profi Cube" index="4" onResetSetCount="0" size="8mm" icon="images/sampleColorProfi.png">\n\
			<group name="CPC-Confusius">\n\
				<fill type="TEXTURE_BLEND" texind="0" k="0" r="111" g="185" b="210" exR="15" exG="146" exB="159"/>\n\
				<pieces copy="PC-Confusius"/> \n\
			</group>\n\
			<group name="CPC-DaVinci">\n\
				<fill type="TEXTURE_BLEND" texind="0" k="0" r="94" g="255" b="65" exR="0" exG="216" exB="173"/>\n\
				<pieces copy="PC-DaVinci"/>\n\
			</group>\n\
			<group name="CPC-MarcoPolo">\n\
				<fill type="TEXTURE_BLEND" texind="0" k="0" r="251" g="251" b="7" exR="0" exG="213" exB="203"/>\n\
				<pieces copy="PC-MarcoPolo"/>\n\
			</group>\n\
			<group name="CPC-Rubens">\n\
				<fill type="TEXTURE_BLEND" texind="0" k="0" r="255" g="114" b="0" exR="255" exG="163" exB="0"/>\n\
				<pieces copy="PC-Rubens"/>\n\
			</group>\n\
			<group name="CPC-Watt">\n\
				<fill type="TEXTURE_BLEND" texind="0" k="0" r="255" g="116" b="0" exR="246" exG="0" exB="0"/>\n\
				<pieces copy="PC-Watt"/>\n\
			</group>\n\
			<group name="CPC-Newton">\n\
				<fill type="TEXTURE_BLEND" texind="0" k="0" r="167" g="20" b="153" exR="255" exG="37" exB="56"/>\n\
				<pieces copy="PC-Newton"/>\n\
			</group>\n\
		</family>\n\
		<family name="Micro Cube" index="8" onResetSetCount="0" size="4mm" icon="images/sampleMicro.png">\n\
			<group name="UC-Barcelona"> <!-- blue -->\n\
				<fill type="COLOR" k="0" r="73" g="129" b="255"/>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>1 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 0 1 0 0</piece>\n\
				<piece>1 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>1 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 1 0 1 1</piece>\n\
				<piece>0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 1</piece>\n\
			</group>\n\
			<group name="UC-Bern"> <!-- green -->\n\
				<fill type="COLOR" k="0" r="43" g="221" b="56"/>\n\
				<piece>1 1 0 1 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 1 0 1 0</piece>\n\
				<piece>1 1 0 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>0 1 0 1 1  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece>0 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 0 1 0 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 1</piece>\n\
			</group>\n\
			<group name="UC-Birmingham"> <!-- yellow -->\n\
				<fill type="COLOR" k="0" r="255" g="255" b="0"/>\n\
				<piece>1 1 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 1</piece>\n\
				<piece>0 1 0 1 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 1</piece>\n\
				<piece>0 0 1 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>1 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 1 0 1 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 0 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 0  1 0 1 0 0</piece>\n\
			</group>\n\
			<group name="UC-Glasgow"> <!-- orange --> \n\
				<fill type="COLOR" k="0" r="255" g="140" n="0"/>\n\
				<piece>1 1 0 1 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  1 1 0 0 0</piece>\n\
				<piece>0 1 0 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 0 1 0 0</piece>\n\
				<piece>1 1 0 1 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>0 0 1 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>1 1 0 1 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 1</piece>\n\
			</group>\n\
			<group name="UC-Manchester"> <!-- red -->\n\
				<fill type="COLOR" k="0" r="255" g="43" b="48"/>\n\
				<piece>0 1 0 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 0 1 0 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece>0 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>1 1 0 1 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 0 1 0 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 0</piece>\n\
				<piece>0 1 0 1 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 1 0 1 0</piece>\n\
			</group>\n\
			<group name="UC-Oxford"> <!-- purple -->\n\
				<fill type="COLOR" k="0" r="148" g="0" b="136"/>\n\
				<piece>1 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 0  1 0 1 0 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  1 1 1 1 0  0 0 1 0 0</piece>\n\
				<piece>1 1 0 1 1  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 1</piece>\n\
				<piece>0 1 0 1 1  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 0 0 1 1</piece>\n\
				<piece>0 1 0 1 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece>0 0 1 0 0  0 1 1 1 1  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
			</group>\n\
		</family>\n\
		<family name="Smart Cube" index="6" onResetSetCount="0" size="10mm" icon="images/sampleBaub.png">\n\
			<group name="SC-Blue"> <!-- blue was r="51" g="204" b="204"-->\n\
				<fill type="COLOR" k="0" r="73" g="129" b="255"/>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece>0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>1 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece>1 1 0 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece>0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
			</group>\n\
			<group name="SC-Green"> <!-- green was r="136" g="183" b="0"-->\n\
				<fill type="COLOR" k="0" r="43" g="221" b="56"/>\n\
				<pieces copy="SC-Blue"/>\n\
			</group>\n\
			<group name="SC-Yellow"> <!-- yellow was r="255" g="204" b="0"-->\n\
				<fill type="COLOR" k="0" r="255" g="255" b="0"/>\n\
				<pieces copy="SC-Blue"/>\n\
			</group>\n\
			<group name="SC-Orange"> <!-- orange was r="255" g="102" n="0" --> \n\
				<fill type="COLOR" k="0" r="255" g="140" n="0"/>\n\
				<pieces copy="SC-Blue"/>\n\
			</group>\n\
			<group name="SC-Red"> <!-- red -->\n\
				<fill type="COLOR" k="0" r="255" g="0" b="0"/>\n\
				<pieces copy="SC-Blue"/>\n\
			</group>\n\
			<group name="SC-Purple"> <!-- purple was r="153" g="51" b="102" -->\n\
				<fill type="COLOR" k="0" r="148" g="0" b="136"/>\n\
				<pieces copy="SC-Blue"/>\n\
			</group>\n\
		</family>\n\
		<family name="Construct" index="7" onResetSetCount="0" size="10mm" icon="images/sampleConstruct.png">\n\
			<group name="C-Blue"> <!-- blue -->\n\
				<fill type="COLOR" k="0" r="73" g="129" b="255"/>\n\
				<piece>0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece>0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece>0 1 0 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece>1 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece>1 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 1</piece>\n\
			</group>\n\
			<group name="C-Green"> <!-- green -->\n\
				<fill type="COLOR" k="0" r="43" g="221" b="56"/>\n\
				<pieces copy="C-Blue"/>\n\
			</group>\n\
			<group name="C-Yellow"> <!-- yellow -->\n\
				<fill type="COLOR" k="0" r="255" g="255" b="0"/>\n\
				<pieces copy="C-Blue"/>\n\
			</group>\n\
			<group name="C-Orange"> <!-- orange --> \n\
				<fill type="COLOR" k="0" r="255" g="140" n="0"/>\n\
				<pieces copy="C-Blue"/>\n\
			</group>\n\
			<group name="C-Red"> <!-- red -->\n\
				<fill type="COLOR" k="0" r="255" g="43" b="48"/>\n\
				<pieces copy="C-Blue"/>\n\
			</group>\n\
			<group name="C-Purple"> <!-- purple -->\n\
				<fill type="COLOR" k="0" r="148" g="0" b="136"/>\n\
				<pieces copy="C-Blue"/>\n\
			</group>\n\
		</family>\n\
		<!-- these pieces are the same as little-genious but with different orientation-->\n\
		<family name="New Little Genius" index="1" onResetSetCount="0" size="8mm" icon="images/sampleNewLittleGenius.png">\n\
			<group name="NLG-Fruit">\n\
				<fill type="TEXTURE_INDIVIDUAL_WHOLE" texind="3" k="0" r="65" g="154" b="232"/>\n\
				<piece x1="32"  y1="32">0 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece x1="160" y1="32">0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece x1="288" y1="32">0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece x1="416" y1="32">0 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece x1="544" y1="32">0 0 1 0 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece x1="672" y1="32">0 1 0 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
			</group>\n\
			<group name="NLG-Animals">\n\
				<fill type="TEXTURE_INDIVIDUAL_WHOLE" texind="3" k="0" r="72" g="222" b="68"/>\n\
				<piece x1="32"  y1="192">1 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece x1="160" y1="192">1 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece x1="288" y1="192">0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 1</piece>\n\
				<piece x1="416" y1="192">1 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece x1="544" y1="192">1 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece x1="672" y1="192">0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 1</piece>\n\
			</group>\n\
			<group name="NLG-Vehicles">\n\
				<fill type="TEXTURE_INDIVIDUAL_WHOLE" texind="3" k="0" r="254" g="184" b="26"/>\n\
				<piece x1="32"  y1="352">0 1 0 1 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece x1="160" y1="352">1 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece x1="288" y1="352">0 0 1 0 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece x1="416" y1="352">0 0 1 0 1  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  0 1 0 1 1</piece>\n\
				<piece x1="544" y1="352">1 1 0 1 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 0 1 0 0</piece>\n\
				<piece x1="672" y1="352">1 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 0</piece>\n\
			</group>\n\
			<group name="NLG-Sports">\n\
				<fill type="TEXTURE_INDIVIDUAL_WHOLE" texind="3" k="0" r="252" g="115" b="35"/>\n\
				<piece x1="32"  y1="512">0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 0 1 0 0</piece>\n\
				<piece x1="160" y1="512">0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 0 1 0 0</piece>\n\
				<piece x1="288" y1="512">0 1 0 1 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 1 0 1 1</piece>\n\
				<piece x1="416" y1="512">0 1 0 1 1  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 0 1 0 0</piece>\n\
				<piece x1="544" y1="512">0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 0</piece>\n\
				<piece x1="672" y1="512">0 1 0 1 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 1 0 1 1</piece>\n\
			</group>\n\
			<group name="NLG-Weather">\n\
				<fill type="TEXTURE_INDIVIDUAL_WHOLE" texind="3" k="0" r="255" g="61" b="61"/>\n\
				<piece x1="32"  y1="672">1 1 0 1 1  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 1</piece>\n\
				<piece x1="160" y1="672">0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece x1="288" y1="672">0 1 0 1 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 0 1 0 0</piece>\n\
				<piece x1="416" y1="672">0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 1 0 1 0</piece>\n\
				<piece x1="544" y1="672">1 1 0 1 1  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece x1="672" y1="672">0 0 1 0 0  1 1 1 1 0  0 1 1 1 1  1 1 1 1 0  0 0 1 0 0</piece>\n\
			</group>\n\
			<group name="NLG-Nature">\n\
				<fill type="TEXTURE_INDIVIDUAL_WHOLE" texind="3" k="0" r="188" g="0" b="177"/>\n\
				<piece x1="32"  y1="832">0 1 0 1 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  1 0 1 0 0</piece>\n\
				<piece x1="160" y1="832">0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece x1="288" y1="832">0 1 0 1 0  1 1 1 1 1  0 1 1 1 0  1 1 1 1 1  0 0 1 0 0</piece>\n\
				<piece x1="416" y1="832">0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
				<piece x1="544" y1="832">0 1 0 1 0  0 1 1 1 1  1 1 1 1 0  0 1 1 1 1  1 1 0 1 0</piece>\n\
				<piece x1="672" y1="832">0 0 1 0 0  0 1 1 1 0  1 1 1 1 1  0 1 1 1 0  1 1 0 1 1</piece>\n\
			</group>\n\
		</family>	\n\
	</outlines>\n\
</pieces>'