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

#ifndef __CONFIGURATION_H__INCLUDED__
#define __CONFIGURATION_H__INCLUDED__

/** \file
	Declares the DisplayConf, EngineConf, Configuration classes which deal with the application configuration.
	these classes save the application configuration parameters to persistant storage using the
	QSettings class from QT. the configuration can be changed by using the OptionsDlg dialog.
*/

class Solutions;
class PicBucket;
class PicsSet;
class Shape;
class Cube;

class QSettings;

enum EPersistSlvSession
{
	PERSIST_ONLY_FIRST = 0,
	PERSIST_ALL = 1,
	PERSIST_UPTO = 2
};

/// controls how to solution engine treats Asymmetric pieces like the ones in the Little Genius cubes
enum EAsymMethod 
{  
	ASYM_REGULAR = 0, ///< treat them like any other pieces
	ASYM_OUTSIDE = 1, ///< find solution so that the image will face outwards
	ASYM_INSIDE = 2  ///< find solutions so that the image will face inwards.
};

/// some pre-configured display configuration sets that are popular
enum EDisplayPreConf
{  
	DPC_MINIMAL_SQUARE,
	DPC_SOFT_NORMAL,
	DPC_SOFT_EXTRA,
	DPC_SOFT_EXTRA_FAST,
	DPC_CUSTOM
};

/// controls how lines are draw in the solution scene, if at all.
enum ELinesDraw
{
	LINES_NONE = 0,
	LINES_ALL = 1,
	LINES_WHITE = 2  // draw lines only between black pieces
};

/** DisplayConf holds configuration parameters for the 3D \b solution display.
	It controls how the pieces look. it they are flat or round, and if round,
	how round exactly. it is used in PicPainter where the piece display lists
	are created. An instance of this class is contained in the Configuration 
	class and hold the current 3D solution display configuration.
	\see Configuration PicPainter OptionsDlg
*/
class DisplayConf
{
public:
	DisplayConf() 
		:numberOfPasses(0), bVtxNormals(false), bLight(false), nLines(LINES_NONE), nSwapTexEndians(-1)
	{
		for(int i = 0; i < 4; ++i)
			passRound[i] = false;
	}
	void toRegistry(QSettings &reg);
	void fromRegistry(QSettings &reg);
	void fromDefaults();

	/// load the solution display configuration from one of a few fixed popular configurations
	void fromPreConf(EDisplayPreConf pc);

	bool diffRender(const DisplayConf& n);
	bool diffOnlyPaint(const DisplayConf& n);

	/// the pre-configuraion used. this value is not automatically detected.
	/// it is set only if the user chose a pre-configuration.
	EDisplayPreConf preConf; 

	int numberOfPasses; ///< possible values: 0, 1, 2, 3, 4
	bool passRound[4]; ///< controls which subdivision passes are processed as "round" and which as "square"
	/// toggle normals per vertex or per polygon. per vertex normals make the mess look more
	/// soft and less polygon-y
	bool bVtxNormals; 
	bool bLight; ///< is lighting enabled in the scene
	ELinesDraw nLines; ///< should border lines be drawn
	int nSwapTexEndians; ///< the texture endianiness permutation selected.

};

/** EngineConf holds configuration parameters for the solution engine.
	It is used mostly in Cube. An instance of this class is contained in the
	Configuration class and hold the current engine configuration. A copy of the
	most recent engine config is stored in an instance of Cube during the 
	run of the solution engine.
	\see Configuration Cube OptionsDlg
*/
class EngineConf
{
public:
	/// real defaults are in fromDefaults()
	EngineConf()
	: fRand(false), fRestart(false), fLuck(false), nRestart(0), nLuck(0), nPersist(PERSIST_ONLY_FIRST),  
	  nUpto(0), fAfter(false), nAfter(0), nAsym(ASYM_REGULAR)
	{} 

	void toRegistry(QSettings &reg);
	void fromRegistry(QSettings &reg);
	void fromDefaults();

	bool fRand;	///< randomize pieces when searching for a solution
	bool fRestart; ///< should we consider making restarts along the way
	bool fLuck; ///< use the luck paremeter to supress restarts
	int nRestart; ///< number of miliseconds between restarts
	int nLuck; ///< percent on the number of pics in the structure to consider 'lucky'
	EPersistSlvSession nPersist; ///< what to do after finding a solution
	int nUpto; ///< if nPresist is PERSIST_UPTO, how many solutions before restart
	bool fAfter; ///< should we stop after nAfter solution all in all.
	int nAfter; ///< after how many solution all in all should we stop

	EAsymMethod nAsym; ///< how asymmetric piece should be treated

};

/** Configuration holds the configuration of the application.
	The application configuration divides to two sections: solution engine
	configuration and 3D solution display configuration. additionally
	nOptionsSelTab is the currently selected tab in the options
	dialog box. When there will be more GUI related parameters the'll be
	refactored to a third section. Further GUI parameters are saved
	by MainWindow
	An instance of this class is contained in CubeDoc and represents
	the current configuration.
	\see OptionsDlg
*/
class Configuration
{
public:
	Configuration()
	: nOptionsSelTab(0)
	{} 

	void fromRegistry();
	void fromDefaults();
	void toRegistry();

	EngineConf engine;
	DisplayConf disp;

	int nOptionsSelTab; ////< the selected tab in the options dialog
};



#endif // __CONFIGURATION_H__INCLUDED__
