/***************************************************************************
 *   Copyright 2005-2008 Last.fm Ltd.                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#include "PlayerBucketWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "PlayerBucketList.h"
#include "widgets/RadioControls.h"
#include "widgets/UnicornWidget.h"
#include "the/radio.h"
#include "PlayableListItem.h"

#include <QPainter>

PlayerBucketWidget::PlayerBucketWidget( QWidget* p )
                   :QWidget( p )
{
    setAutoFillBackground( true );
    new QHBoxLayout( this );
    
    layout()->setContentsMargins( 0, 0, 0, 0 );
    
    struct BorderedContainer : public QWidget
    {
        BorderedContainer( QWidget* parent = 0 ) : QWidget( parent ){ setAutoFillBackground( true ); }
        
        void setWidget( QWidget* w ){ m_widget = w; w->show();}
        void paintEvent( QPaintEvent* e )
        {
            QPainter p( this );
            
            p.setClipRect( e->rect());
            p.setRenderHint( QPainter::Antialiasing, true );
            QPen pen( QBrush(0x4e4e4e), 3, Qt::DashLine, Qt::RoundCap );
            pen.setDashPattern( QVector<qreal>() << 5 << 2 );
            pen.setWidth( 1 );
            p.setPen( pen );
            p.drawRoundedRect( rect().adjusted( 10, 10, -10, -10), 10, 10 );
        }
        
        void resizeEvent( QResizeEvent* e )
        {
            QLinearGradient g( 0, 0, 0, height() );
            g.setColorAt( 0, 0x070707 );
            g.setColorAt( 1, 0x1b1a1a );
            
            QPalette p = palette();
            p.setBrush( QPalette::Window, g );
            setPalette( p );
            
            QSize size = e->size() - QSize( 30, 30 );
            m_widget->resize( size );
        }
        
        void moveEvent( QMoveEvent* e )
        {
            QPoint widgetPos = e->pos() + QPoint( 15, 15 );
            m_widget->move( widgetPos );
        }
        
        QWidget* m_widget;
    };
    
    ((QBoxLayout*)layout())->addStretch( 0 );
    layout()->addWidget( ui.controls = new RadioControls );
    ui.controls->setAutoFillBackground( false );
    
    QWidget* bucketLayoutWidget = new QWidget( this );

    BorderedContainer* bc;
    ((QBoxLayout*)layout())->addStretch( 2 );
    layout()->addWidget( bc = new BorderedContainer( this ) );
    bc->setWidget( ui.bucket = new PlayerBucketList( bucketLayoutWidget ) );
    bc->setFixedSize( 200, 100 );
    ((QBoxLayout*)layout())->addStretch( 3 );  
  
    
//    layout()->addWidget( bucketLayoutWidget );
    ((QBoxLayout*)layout())->insertStretch( -1, 1);
    
    connect( ui.bucket, SIGNAL( itemRemoved( QString, Seed::Type)), SIGNAL( itemRemoved( QString, Seed::Type)));
    
    
    connect( ui.controls, SIGNAL( skip()), &The::radio(), SLOT( skip()) );
    connect( ui.controls, SIGNAL( stop()), &The::radio(), SLOT( stop()));
    connect( ui.controls, SIGNAL( play()), ui.bucket, SLOT( play()));
    
    UnicornWidget::paintItBlack( this );
    UnicornWidget::paintItBlack( bucketLayoutWidget);
}


void 
PlayerBucketWidget::addAndLoadItem( const QString& itemText, const Seed::Type type )
{
    PlayableListItem* item = new PlayableListItem;
    item->setText( itemText );
    item->setPlayableType( type );
	item->setForeground( Qt::white );
	item->setBackground( QColor( 0x2e, 0x2e, 0x2e));
	item->setFlags( item->flags() | Qt::ItemIsDragEnabled );
    item->fetchImage();
    ui.bucket->addItem( item );
    
}


void 
PlayerBucketWidget::resizeEvent( QResizeEvent* event )
{
    QPalette p = palette();
    QLinearGradient lg( 0, 0, 0, event->size().height() );
    lg.setColorAt( 0, 0x5c5e60 );
    lg.setColorAt( 1, 0x353536 );
    p.setBrush( QPalette::Window, lg );
    setPalette( p );
}