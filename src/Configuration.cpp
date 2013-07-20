// Happy Cube Solver - Building and designing models for the Happy cube puzzles
// Copyright (C) 1995-2006 Shy Shalom, shooshX@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "Configuration.h"
#include "colorFuncs.h"

#include <QSettings>

void Configuration::fromRegistry()
{
	QSettings reg("Happy Cube Solver", "Happy Cube Solver (QT)");
	engine.fromRegistry(reg);
	disp.fromRegistry(reg); 
	
	reg.beginGroup("GUI");
	nOptionsSelTab = reg.value("nOptionsSelTab", 2).toInt(); // default is 2 - display
	reg.endGroup();
}

void Configuration::fromDefaults()
{
	engine.fromDefaults();
	disp.fromDefaults();
	nOptionsSelTab = 2;
}
	
void Configuration::toRegistry()
{	
	QSettings reg("Happy Cube Solver", "Happy Cube Solver (QT)");
	engine.toRegistry(reg);
	disp.toRegistry(reg);

	reg.beginGroup("GUI");
	reg.setValue("nOptionsSelTab", nOptionsSelTab);
	reg.endGroup();
}



void EngineConf::toRegistry(QSettings &reg)
{

	reg.beginGroup("Options");
	reg.setValue("fRand", (int)fRand);
	reg.setValue("fRestart", (int)fRestart);
	reg.setValue("fLuck", (int)fLuck);
	reg.setValue("nLuck", nLuck);
	reg.setValue("nPersist", (int)nPersist);
	reg.setValue("nRestart", nRestart);
	reg.setValue("nUpto", nUpto);
	reg.setValue("fAfter", (int)fAfter);

	reg.setValue("nAsym", (int)nAsym);
	reg.setValue("nAfter", (int)nAfter);
	reg.endGroup();
	
	
}


void EngineConf::fromDefaults()
{
	fRand = true; 
	fRestart = true;
	fLuck = true;
	nLuck = 8;	// percent
	nPersist = PERSIST_UPTO;	
	nRestart = 2000; // miliseconds
	nUpto = 5;	
	fAfter = true;
	nAfter = 5;

	nAsym = ASYM_REGULAR;
}

void EngineConf::fromRegistry(QSettings &reg)
{

	reg.beginGroup("Options");
	int frnd = reg.value("fRand", -1).toInt();
	if (frnd == -1)
	{ // no registry entry, use defaults
		reg.endGroup();
		fromDefaults();
		toRegistry(reg);
		return;
	}

	fRand = frnd != 0;
	fRestart =  reg.value("fRestart", -1).toInt() != 0;
	fLuck =  reg.value("fLuck", -1).toInt() != 0;
	nLuck =  reg.value("nLuck", -1).toInt();
	nPersist =  (EPersistSlvSession)reg.value("nPersist", -1).toInt();
	nRestart =  reg.value("nRestart", -1).toInt();
	nUpto =  reg.value("nUpto", -1).toInt();
	fAfter = reg.value("fAfter", -1).toInt() != 0;

	nAsym = (EAsymMethod)reg.value("nAsym", -1).toInt();
	if (nAsym == -1) nAsym = ASYM_REGULAR; // new parameter could be not there

	nAfter = reg.value("nAfter", -1).toInt();
	if (nAfter == -1) nAfter = 5; // new parameter could be not there
	reg.endGroup();

}


void DisplayConf::fromPreConf(EDisplayPreConf pc)
{
	preConf = pc;
	switch (pc)
	{
	case DPC_MINIMAL_SQUARE:
		numberOfPasses = 0;
		passRound[0] = false; passRound[1] = false;
		passRound[2] = false; passRound[3] = false;
		bVtxNormals = false;
		bLight = false;
		nLines = LINES_ALL;
		break;
	case DPC_SOFT_NORMAL:
		numberOfPasses = 3;
		passRound[0] = false; passRound[1] = false;
		passRound[2] = true;  passRound[3] = true;
		bVtxNormals = true;
		bLight = true;
		nLines = LINES_WHITE;
		break;
	case DPC_SOFT_EXTRA:
		numberOfPasses = 3;
		passRound[0] = false; passRound[1] = true;
		passRound[2] = true;  passRound[3] = true;
		bVtxNormals = true;
		bLight = true;
		nLines = LINES_WHITE;
		break;
	case DPC_SOFT_EXTRA_FAST:
		numberOfPasses = 2;
		passRound[0] = false; passRound[1] = true;
		passRound[2] = true;  passRound[3] = true;
		bVtxNormals = true;
		bLight = true;
		nLines = LINES_WHITE;
		break;
	case DPC_CUSTOM:
		break; // do nothing. stay with last values
	}
}

void DisplayConf::fromDefaults() 
{ 
	fromPreConf(DPC_SOFT_NORMAL); 
	nSwapTexEndians = 1234;
}

void DisplayConf::toRegistry(QSettings &reg)
{
	reg.beginGroup("Display");
	reg.setValue("preConf", (int)preConf);
	reg.setValue("numberOfPasses", (int)numberOfPasses);
	for (int i = 0; i < 4; ++i)
		reg.setValue(QString("passRound%1").arg(i), (int)passRound[i]);
	reg.setValue("bVtxNormals", (int)bVtxNormals);
	reg.setValue("bLight", (int)bLight);
	reg.setValue("nLines", (int)nLines);

	reg.setValue("nSwapTexEndians", int(nSwapTexEndians));
	reg.setValue("slvBkColor", vec3ToText(slvBkColor));
	reg.endGroup();
}

void DisplayConf::fromRegistry(QSettings &reg)
{
	reg.beginGroup("Display");
	int iprec = reg.value("preConf", -1).toInt();
	if (iprec == -1)
	{
		reg.endGroup();
		fromDefaults();
		toRegistry(reg);
		return;
	}
	preConf = (EDisplayPreConf)iprec;
	numberOfPasses = reg.value("numberOfPasses", -1).toInt();
	for (int i = 0; i < 4; ++i)
		passRound[i] = (bool)reg.value(QString("passRound%1").arg(i), 0).toInt();
	bVtxNormals = (bool)reg.value("bVtxNormals", 0).toInt();
	bLight = (bool)reg.value("bLight", 0).toInt();
	nLines = (ELinesDraw)reg.value("nLines", LINES_WHITE).toInt(); // might not exist, LINES_BLACK is default

	slvBkColor = vec3FromText(reg.value("slvBkColor", "").toString());

	int swapRes = reg.value("nSwapTexEndians", -1).toInt();
	if (swapRes == -1)
	{
#ifdef Q_WS_MAC
		nSwapTexEndians = 1234; // on Mac it needs to be true
#else
		nSwapTexEndians = 1234;
#endif
	}
	else
		nSwapTexEndians = swapRes;
	reg.endGroup();

}



// is there a need to rebuild the objects with the new conf?
bool DisplayConf::diffRender(const DisplayConf& n)
{
	bool ret = (numberOfPasses != n.numberOfPasses) || (bVtxNormals != n.bVtxNormals);
	for (int i = 0; i < 4; ++i)
		ret = ret || (passRound[i] != n.passRound[i]);
	return ret;
}
bool DisplayConf::diffOnlyPaint(const DisplayConf& n)
{
	return (bLight != n.bLight) || (nLines != n.nLines);
}
