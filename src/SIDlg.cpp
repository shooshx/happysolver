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

#include "SIDlg.h"
#include "Pieces.h"

#include <QPainter>
#include <QTimer>
#include <QMatrix>
#include <QSettings>
#include <QPixmap>
#include <QBitmap>
#include <QDateTime>
#include <QMouseEvent>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>

#define SOSER_X_SIZE 170
#define SOSER_Y_SIZE 43


void GameWidget::Shot::moveUp(int dy)
{ 
    reg.translate(0, -dy); 
    y -= dy;
}

GameWidget::Ship::Ship(int _x, int _y, int _row, int _col, const QPixmap *p)
:x(_x), y(_y), row(_row), col(_col), pix(p), reg(pix->mask())
{
    reg.translate(x, y);
}

void GameWidget::Ship::move(int dx, int dy)
{
    x += dx; y += dy;
    reg.translate(dx, dy);
}

GameWidget::GameWidget(QWidget *parent)
:QLabel(parent), m_gunX(32), m_shipXHeading(1), m_soser(250, 65)
{
    srand(QDateTime::currentDateTime().toTime_t());

    m_shipTimeInt = 200.0;

    m_shottimer = new QTimer(this);
    connect(m_shottimer, SIGNAL(timeout()), this, SLOT(shotTimeout()));
    m_shottimer->start(30);
    m_shiptimer = new QTimer(this);
    connect(m_shiptimer, SIGNAL(timeout()), this, SLOT(shipTimeout()));
    m_shiptimer->start((int)m_shipTimeInt);
    m_soserShowTimer = new QTimer(this);
    connect(m_soserShowTimer, SIGNAL(timeout()), this, SLOT(soserShowTimeout()));
    m_soserShowTimer->setSingleShot(false);
    int tm = (((rand() % 6) + 5) * 1000);
    m_soserShowTimer->start(tm);
    m_soserMoveTimer = new QTimer(this);
    connect(m_soserMoveTimer, SIGNAL(timeout()), this, SLOT(soserMoveTimeout()));
    m_levelFinTimer = new QTimer(this);
    connect(m_levelFinTimer, SIGNAL(timeout()), this, SLOT(levelFinTimeout()));

    setMouseTracking(true);

    // draw soser pixmap
    m_soser.fill(Qt::transparent);
    QPainter p(&m_soser);
    QPixmap a = PicBucket::instance().getPic(4, 0).pixmap;
    QPixmap b = PicBucket::instance().getPic(4, 3).pixmap;
    p.drawPixmap(0, 0, a);
    p.drawPixmap(51, 0, b.transformed(QMatrix().rotate(270), Qt::SmoothTransformation).transformed(
        QMatrix(QMatrix(1, 0, 0, -1, 0, 0)), Qt::SmoothTransformation)); // flip it, looks better
    p.drawPixmap(102, 0, b);
    p.drawPixmap(153, 0, a.transformed(QMatrix(-1, 0, 0, 1, 0, 0), Qt::SmoothTransformation));
    p.end();
    m_soser = m_soser.scaled(QSize(170, 43), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_soserMask = QRegion(m_soser.mask());

    m_score = 0;
    m_level = 1;
    m_lives = 3;
    m_soserX = 0;

    sizeY = 9999;

    loadScores();
    initLevel(m_level);

}

void GameWidget::initLevel(int level)
{
    int salt = 0;
    m_ships.clear();

    int startY = qMin(20 + (level - 1) * 20, sizeY - 150 - 4 * 70);
    for (int row = 0; row < 4; ++row)
    {
        for(int col = 0; col < 10; ++col)
        {
            int def = (((level - 1) % 5) * 6) + ((row+col*level))%6;
            int pic = (row*col+salt)%6;
            m_ships.push_back(Ship(col * 75, startY + row * 70, row, col, &PicBucket::instance().getPic(def, pic).pixmap));
            salt += (rand() % 4);
        }
    }
    m_shots.clear();
    update(); // invalidate the whole screen

    m_shipTimeInt = 200;
    m_shipXHeading = 1;
    m_shipYheading = 0;
    m_gunY = sizeY - 65;
    m_bInteract = true;
    m_bSoserEnabled = true;
    stopSoser();

    emit levelChanged(level);
}


void GameWidget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    if (m_bShowSoser)
    {
        p.drawPixmap(m_soserX, 0, m_soser);
    }

    for(TShips::const_iterator sit = m_ships.constBegin(); sit != m_ships.constEnd(); ++sit)
    {
        p.drawPixmap(sit->x, sit->y, *sit->pix);
    }

    QRadialGradient grad1(0, 0, 7);
    grad1.setColorAt(1, QColor(0,0,0));
    grad1.setColorAt(0, QColor(255,255,255));
    QPen pen1(QColor(0,0,0));
    QRadialGradient grad2(0, 0, 7);
    grad2.setColorAt(1, QColor(255,0,0));
    grad2.setColorAt(0, QColor(255,200,200));
    QPen pen2(QColor(255,0,0));
    
    p.setPen(pen1);
    p.setBrush(QBrush(grad1));
    int lastL = 1;

    for(TShots::const_iterator it = m_shots.constBegin(); it != m_shots.constEnd(); ++it)
    {
        int x = it->x, y = it->y, l = it->level;
        if (l != lastL)
        {
            p.setPen((l==1)?pen1:pen2);
            p.setBrush((l==1)?grad1:grad2);
            lastL = l;
        }
        p.setBrushOrigin(x, y);
        p.drawEllipse(x - 7, y - 7, 14, 14);
    }

    p.drawPixmap(m_gunX-32, m_gunY, PicBucket::instance().getPic(1, 2).pixmap);
    m_shotspreg = QRegion();

    if (m_lives == 0)
    {	
        p.setPen(QPen(Qt::NoPen));
        p.setBrush(QBrush(QColor(255,255,255,188)));
        p.drawRect(0, 0, sizeX, sizeY);
        QLabel::paintEvent(e);
        return;
    }
}

void GameWidget::shotTimeout()
{
    bool paint = false;
    bool erase;

    TShots::iterator it = m_shots.begin();
    while(it != m_shots.constEnd())
    {
        erase = false;

        if (it->y < - 20)
            erase = true;
        else
        {
            int hitfactor = (it->level > 1)?100:1;
            TShips::iterator ship = m_ships.begin();
            while (ship != m_ships.end())
            {
                if (!ship->reg.intersects(it->reg))
                {
                    m_shotspreg += QRegion(ship->x, ship->y, 65, 65);
                    m_ships.erase(ship);

                    m_score += 10 * hitfactor;
                    emit scoreChanged(m_score);
                    m_shipTimeInt -= (float)4.9;

                    if (it->level == 1)
                        erase = true;
                    break; // out of the ships search loop
                }
                ++ship;
            }
            if (m_bShowSoser && (!m_soserMask.intersects(it->reg)))
            {
                stopSoser();
                m_shotspreg += QRegion(m_soserX, 0, m_soserX + SOSER_X_SIZE + 7, SOSER_Y_SIZE);
                m_score += 25 * hitfactor;
                emit scoreChanged(m_score);
                erase = true;
            }
        }

        it->moveUp(15);
        int x = it->x, y = it->y;

        paint = true;
        m_shotspreg += QRegion(x - 10, y - 10, 20, 35);

        if (erase)
            it = m_shots.erase(it);
        else
            ++it;
    }

    update(m_shotspreg);

}

void GameWidget::die()
{
    --m_lives;
    emit livesChanged(m_lives);
    if (m_lives == 0)
    {
        m_bInteract = false;
        m_shottimer->stop();
        m_shiptimer->stop();
        m_soserMoveTimer->stop();
        m_soserShowTimer->stop();
        m_shotspreg += QRegion(0, 0, sizeX, sizeY); // invalidate the whole screen
        update(); // invalidate all
        endGame();
    }
    else
    {
        initLevel(m_level);
    }
}

void GameWidget::shipTimeout()
{
    int nextHead = 0; // 0 means nothing changed
    int mvX, mvY;
    bool reachedBottom = false;

    for(TShips::iterator sit = m_ships.begin(); sit != m_ships.end(); ++sit)
    {
        int x = sit->x, y = sit->y;
        QRegion r(x, y, 65, 65);
        m_shotspreg += r;

        if (m_shipYheading != 0)
        { // head down
            mvY = 30;
            mvX = 0;
        }
        else
        {
            mvY = 0;
            mvX = m_shipXHeading * 7;

            if (x + mvX + 65 + 20 >= sizeX)
                nextHead = -1;
            else if (x + mvX < 20)
                nextHead = 1;
        }

        if (y + mvY + (65 * 2)-10 >= sizeY)
        { // need to do this check every time since the window might have resized
            reachedBottom = true; // wait for the end to take action (safer.. don't half bake view)
        }
        
        sit->move(mvX, mvY);
        r.translate(mvX, mvY);
        m_shotspreg += r;
    }

    m_shipYheading = 0;
    if ((nextHead != 0) && (nextHead != m_shipXHeading))
    { // we actually changed direction
        m_shipXHeading = nextHead;
        m_shipYheading = 1;
    }

    if (m_ships.empty() && (!m_levelFinTimer->isActive()))
    {
        m_levelFinTimer->start(30);
    }

    if (m_shiptimer->interval() != m_shipTimeInt)
    {
        m_shiptimer->setInterval((int)m_shipTimeInt);
    }

    update(m_shotspreg);

    if (reachedBottom)
    {
        die();
    }

}


void GameWidget::soserShowTimeout()
{
    if (m_bShowSoser || (!m_bSoserEnabled))
        return; // it's already showing
    m_soserMask.translate(-m_soserX - 170, 0);
    m_soserX = -170;
    m_bShowSoser = true;
    m_soserMoveTimer->start(20);
}

void GameWidget::levelFinTimeout()
{
    if (!m_shots.isEmpty()) // wait for all the shots to clear
        return;
    if (m_bShowSoser) // wait for the soser to pass
        return;
    m_bInteract = false;
    m_bSoserEnabled = false;
    if (m_gunY > -65)
    {
        m_shotspreg += QRegion(m_gunX - 32, m_gunY - 10, 65, 65 + 10);
        m_gunY -= 10;
        return;
    }
    else
    {
        m_levelFinTimer->stop();
        initLevel(++m_level);
    }

}


void GameWidget::stopSoser()
{
    if (!m_bShowSoser)
        return;
    m_bShowSoser = false;
    m_soserMoveTimer->stop();
}

void GameWidget::soserMoveTimeout()
{
    m_shotspreg += QRegion(m_soserX, 0, m_soserX + SOSER_X_SIZE + 7, SOSER_Y_SIZE);
    m_soserX += 7;
    m_soserMask.translate(7, 0);
    if (m_soserX >= sizeX)
    {
        stopSoser();
    }
}


void GameWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_bInteract)
        return;
    if (e->x() != m_gunX)
    {
        m_shotspreg += QRegion(m_gunX-32, sizeY-65, 65, 65);
        m_gunX = e->x();
        m_shotspreg += QRegion(m_gunX-32, sizeY-65, 65, 65);
    }
}

void GameWidget::mousePressEvent(QMouseEvent *e)
{
    if (!m_bInteract)
        return;
    int l = (e->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier))?2:1;
    m_shots.push_back(Shot(m_gunX, sizeY-60, l));
}

void GameWidget::resizeEvent(QResizeEvent *e)
{
    sizeX = e->size().width();
    sizeY = e->size().height();
    m_gunY = sizeY - 65;
}

void GameWidget::endGame()
{
    if (m_score >= m_scores.last().score)
    {
        QString name = QInputDialog::getText(this, "Happy Cube Space Invaders!", 
            "<b style=\"font-size:24pt\">What is your name ?!");
        m_scores.append(Score(name, m_score));
        qSort(m_scores);
        m_scores.removeLast();
        saveScores();
    }

    setTextFormat(Qt::RichText);
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QString msg("<font face=\"Courier\"><b style=\"font-size:72pt\">Game Over!</b><br><b style=\"font-size:24pt\">");
    for (TScoreList::const_iterator it = m_scores.constBegin(); it != m_scores.constEnd(); ++it)
    {
        msg += QString("%1.......%2<br>").arg(it->name, -22, '.').arg(humanCount(it->score).c_str(), 13, QChar('.'));
    }
    msg += "</font>";

    setText(msg);
}

bool operator<(const GameWidget::Score &a, const GameWidget::Score &b)
{
    return a.score > b.score; // reverse order
}

void GameWidget::loadScores()
{
    QSettings reg("Happy Cube Solver", "Happy Cube Solver (QT)");

    reg.beginGroup("SI");
    QStringList keys = reg.allKeys();
    if (keys.isEmpty())
    {
        loadDefaultScores();
        reg.endGroup();
        saveScores();
        return;
    }

    m_scores.clear();
    for(QStringList::const_iterator it = keys.constBegin(); it != keys.constEnd(); ++it)
    {
        m_scores.append(Score(*it, reg.value(*it, -1).toInt()));
    }
    qSort(m_scores);

    reg.endGroup();
}

void GameWidget::loadDefaultScores()
{
    m_scores.append(Score("Jean-Luc Picard", 10000000));//1
    m_scores.append(Score("William T. Riker", 5000000));//2
    m_scores.append(Score("Data",             1000000));//3
    m_scores.append(Score("Deanna Troi",       500000));//4
    m_scores.append(Score("Geordi La Forge",   100000));//5
    m_scores.append(Score("Worf",               50000));//6
    m_scores.append(Score("Beverly Crusher",    10000));//7
    m_scores.append(Score("Tasha Yar",           5000));//8
    m_scores.append(Score("Reginald Barclay",    1000));//9
    m_scores.append(Score("Wesley Crusher",       500));//10
}

void GameWidget::saveScores()
{
    QSettings reg("Happy Cube Solver", "Happy Cube Solver (QT)");

    reg.beginGroup("SI");
    reg.remove(""); // erase all current group
    for (TScoreList::const_iterator it = m_scores.constBegin(); it != m_scores.constEnd(); ++it)
    {
        reg.setValue(it->name, (int)it->score); 
    }
    reg.endGroup();
}


SIDlg::SIDlg(QWidget *parent)
:QDialog(parent)
{
    setMinimumSize(450, 450);

    setWindowState((windowState() & ~(Qt::WindowMinimized | Qt::WindowFullScreen)) | Qt::WindowMaximized);

    setSizeGripEnabled(true);
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
    setWindowTitle("Happy Cube Space Invaders!");

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    m_game = new GameWidget(this);
    m_game->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(m_game, SIGNAL(levelChanged(int)), this, SLOT(setLevel(int)));
    connect(m_game, SIGNAL(scoreChanged(int)), this, SLOT(setScore(int)));
    connect(m_game, SIGNAL(livesChanged(int)), this, SLOT(updateLives(int)));

    layout->addWidget(m_game);

    m_lowbar = new QHBoxLayout;
    layout->addLayout(m_lowbar);
    m_level = new QLabel("Level: 1");
    m_score = new QLabel("Score: 0");

    m_lowbar->addWidget(m_score);
    m_lowbar->addStretch();
    m_lowbar->addWidget(m_level);

    QFont font("Courier", 20, QFont::Bold);
    m_level->setFont(font);
    m_score->setFont(font);

    m_lifePix = PicBucket::instance().getPic(1, 2).pixmap.scaled(QSize(25, 25), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    updateLives(3);
}

void SIDlg::setScore(int score)
{
    m_score->setText(QString("Score: %1").arg(score));
}

void SIDlg::setLevel(int level)
{
    m_level->setText(QString("Level: %1").arg(level));
}

void SIDlg::updateLives(int lives)
{
    if (lives <0)
        return;
    if (m_lives.size() < lives)
    {
        while(lives > m_lives.size())
        {
            QLabel *life = new QLabel();
            life->setPixmap(m_lifePix);
            m_lowbar->addWidget(life);
            m_lives.push_back(life);
        }
    }
    else
    {
        while(m_lives.size() > lives)
        {
            QLabel *life = m_lives.back();
            delete life;
            m_lives.pop_back();
        }
    }

}
