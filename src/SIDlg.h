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

#ifndef __SIDLG_H_INCLUDED__
#define __SIDLG_H_INCLUDED__

#include <QLabel>
#include <QDialog>
#include <QRegion>
#include <QList>
#include <QPixmap>

/** \file
	Declares the SIDlg and it's related GameWidget class.
*/

/** GameWidget impelents the easter egg game functionality.
	The game has a single gun ship at the bottom of the screen and multiple 
	ships in a pattern above it.
	from time to time a soser (Saucer) hovers above it all.
*/
class GameWidget :public QLabel
{
	Q_OBJECT
public:
	GameWidget(QWidget *parent);
	virtual ~GameWidget() {}

protected:
	virtual void paintEvent(QPaintEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void resizeEvent(QResizeEvent *e);

private slots:
	void shotTimeout();
	void shipTimeout();
	void soserShowTimeout();
	void soserMoveTimeout();
	void levelFinTimeout();

signals:
	void levelChanged(int level);
	void scoreChanged(int score);
	void livesChanged(int lives);

private:
	void die();
	void initLevel(int level);
	void stopSoser();
	void endGame();
	void loadScores();
	void loadDefaultScores();
	void saveScores();

	QTimer *m_shottimer;
	QTimer *m_shiptimer;
	QTimer *m_soserShowTimer;
	QTimer *m_soserMoveTimer;
	QTimer *m_levelFinTimer;
	
	/// Ship represents a single space invader.
	struct Ship
	{
		Ship(int _x, int _y, int _row, int _col, const QPixmap *p);
		void move(int x, int y);
		int x, y;
		int row, col;
		const QPixmap *pix;
		QRegion reg;
	};
	typedef QList<Ship> TShips;
	TShips m_ships;

	/// Shot represents a single shot shot from the gun.
	struct Shot
	{
		Shot(int _x, int _y, int l) :x(_x), y(_y), reg(x - 7, y - 7, 14, 14, QRegion::Ellipse), level(l) {}
		void moveUp(int dy);
		int x, y;
		QRegion reg;
		int level;
	};
	typedef QList<Shot> TShots;
	TShots m_shots;

	int m_gunX, m_gunY; // x position of the gun (x of the shot)
	int m_shipXHeading, m_shipYheading;
	int sizeX, sizeY;

	QPixmap m_soser;
	QRegion m_soserMask;
	bool m_bShowSoser;
	int m_soserX;

	QRegion m_shotspreg;

	int m_level;
	int m_score;
	int m_lives;

	float m_shipTimeInt;

	bool m_bInteract;
	bool m_bSoserEnabled;

public:
	/// Score is a single entity with a name and a score.
	struct Score
	{
		Score(const QString& _name, int _score) :name(_name), score(_score) {}
		QString name;
		int score;
	};
private:
	typedef QList<Score> TScoreList;
	TScoreList m_scores;

};

class QLabel;
class QHBoxLayout;

/** SIDlg is an easter egg Space Invaders game.
	The easter egg is activated from CubeDoc.
	Most of the game is implemented in GameWidget.
*/
class SIDlg :public QDialog
{
	Q_OBJECT
public:
	SIDlg(QWidget *parent);
	virtual ~SIDlg() {}

public slots:
	void setScore(int score);
	void setLevel(int level);
	void updateLives(int lives);

private:
	GameWidget *m_game;
	QHBoxLayout *m_lowbar;
	QPixmap m_lifePix;

	QLabel *m_level, *m_score;
	QList<QLabel*> m_lives;
};

#endif //__SIDLG_H_INCLUDED__
