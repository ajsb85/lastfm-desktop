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

#include <QListWidget>
#include <QSplitter>
#include <QStackedWidget>
#include <QStackedLayout>
#include <QLineEdit>
#include <QComboBox>
#include "widgets/ImageButton.h"
#include <QPushButton>
#include "widgets/UnicornTabWidget.h"
#include "widgets/UnicornWidget.h"
#include <QStringListModel>
#include "Sources.h"
#include "SourcesList.h"
#include "Amp.h"
#include "the/mainWindow.h"
#include "lib/lastfm/radio/RadioStation.h"
#include "PlayableListItem.h"
#include "DelegateDragHint.h"
#include "widgets/RadioControls.h"
#include "lib/lastfm/types/User.h"
#include "lib/lastfm/ws/WsReply.h"
#include "lib/lastfm/ws/WsAccessManager.h"
#include "widgets/Firehose.h"
#include "radio/buckets/PlayerBucketList.h"
#include <phonon/volumeslider.h>

struct SpecialWidget : QWidget
{
    virtual void paintEvent( QPaintEvent* )
    {
        QPainter p( this );
        p.fillRect( rect(), QColor( 35, 35, 35 ) );
        p.setPen( QColor( 27, 27, 27 ) );
        p.drawLine( 0, 0, width(), 0 );
        p.setPen( QColor( 55, 55, 55 ) );
        p.drawLine( 0, 1, width(), 1 );
    }    
};


Sources::Sources()
        :m_connectedAmp( 0 )
{
    m_accessManager = new WsAccessManager( this );
    
    setupUi();
    
    AuthenticatedUser authUser;
     
    QString const name = authUser.name();
    
    typedef QPair<QString, QString> StringPair;
    typedef QList<StringPair > PairList;
    StringPair station;
    foreach( station, PairList() << StringPair( tr("Your Library"), QString("library:%1").arg(name))
                                 << StringPair( tr("Your\xa0Loved Tracks"), QString("loved:%1").arg(name))
                                 << StringPair( tr("Recommended"), QString( "recs:%1" ).arg(name)))
                               // FIXME: Neighbours not implemented in RQL yet!
                               //  << StringPair( tr("My Neighbours"), QString( "neighbours:%1" ).arg(name)))

    {
        PlayableListItem* n = new PlayableListItem( station.first, ui.stationsBucket );
        n->setRQL( station.second );
        n->setPlayableType( Seed::PreDefinedType );
        n->setSizeHint( QSize( 90, n->sizeHint().height()+20));
        connect( authUser.getInfo(), SIGNAL( finished( WsReply*)), SLOT( onAuthUserInfoReturn( WsReply* )) );
    }
    
    onTabChanged();

    connect( authUser.getFriends(), SIGNAL(finished( WsReply* )), SLOT(onUserGetFriendsReturn( WsReply* )) );
    connect( authUser.getTopTags(), SIGNAL(finished( WsReply* )), SLOT(onUserGetTopTagsReturn( WsReply* )) );
    connect( authUser.getPlaylists(), SIGNAL(finished( WsReply* )), SLOT(onUserGetPlaylistsReturn( WsReply* )) );
    connect( authUser.getTopArtists(), SIGNAL(finished( WsReply* )), SLOT(onUserGetArtistsReturn( WsReply* )) );
	
}


void 
Sources::setupUi()
{
    new QVBoxLayout( this );
    layout()->setMargin( 0 );
    layout()->setSpacing( 0 );
    
    ui.tabWidget = new Unicorn::TabWidget;
    ui.tabWidget->bar()->setTearable( true );

    ui.tabWidget->bar()->addWidget( ui.cog = new ImageButton( ":/MainWindow/button/cog/up.png" ) );
    ui.tabWidget->bar()->addWidget( ui.dashboard = new QPushButton( "Now Playing" ));
    connect( ui.cog, SIGNAL(pressed()), SLOT(onCogMenuClicked()) );
    m_cogMenu = new QMenu( this );
    
    connect( ui.tabWidget, SIGNAL( currentChanged( int )), SLOT( onTabChanged()));
    connect( this, SIGNAL( customContextMenuRequested( const QPoint& )), SLOT( onContextMenuRequested( const QPoint& )));
    
    layout()->addWidget( ui.tabWidget );
    
    ui.stationsBucket = new SourcesList( this );
    ui.stationsBucket->setWindowTitle( tr( "Stations" ) );
    connect( ui.stationsBucket, SIGNAL( doubleClicked(const QModelIndex&)), SLOT( onItemDoubleClicked( const QModelIndex&)));
    UnicornWidget::paintItBlack( ui.stationsBucket );    //on mac, qt 4.4.1 child widgets aren't inheritting palletes properly
    ui.tabWidget->addTab( ui.stationsBucket );
    
    ui.friendsBucket = new SourcesList( this );
    ui.friendsBucket->setWindowTitle( tr( "Friends" ));
    
    Firehose* hose;
    ui.friendsBucket->addCustomWidget( hose = new Firehose, tr( "Firehose View" ) );
    ui.friendsBucket->setSourcesViewMode( SourcesList::CustomMode );

    connect( ui.friendsBucket, SIGNAL( doubleClicked(const QModelIndex&)), SLOT( onItemDoubleClicked( const QModelIndex&)));
    UnicornWidget::paintItBlack( ui.friendsBucket );    //as above
    ui.tabWidget->addTab( ui.friendsBucket );
    
	ui.tagsBucket = new SourcesList( this );
    ui.tagsBucket->setWindowTitle( tr( "Tags" ));
    
    connect( ui.tagsBucket, SIGNAL( doubleClicked(const QModelIndex&)), SLOT( onItemDoubleClicked( const QModelIndex&)));
    UnicornWidget::paintItBlack( ui.tagsBucket );    //as above
    ui.tabWidget->addTab( ui.tagsBucket );

	ui.artistsBucket = new SourcesList( this );
    ui.artistsBucket->setWindowTitle( tr( "Artists" ));
    connect( ui.artistsBucket, SIGNAL( doubleClicked(const QModelIndex&)), SLOT( onItemDoubleClicked( const QModelIndex&)));
    UnicornWidget::paintItBlack( ui.artistsBucket );    //as above
    ui.tabWidget->addTab( ui.artistsBucket );
    
#if 0
    //
    //TODO: Uncomment when we have a recentArtists webservice
    //
    
    QAction* topArtists = new QAction( tr( "Top Artists" ), ui.artistsBucket );
    topArtists->setCheckable( true );
    topArtists->setChecked( true );
    
    QAction* recentArtists = new QAction( tr( "Recent Artists" ), ui.artistsBucket );
    recentArtists->setCheckable( true );
    
    QActionGroup* artistGroup = new QActionGroup( ui.artistsBucket );
    artistGroup->addAction( topArtists );
    artistGroup->addAction( recentArtists );
    
    QAction* sep = new QAction( ui.artistsBucket );
    sep->setSeparator( true );

    ui.artistsBucket->addAction( sep );
    ui.artistsBucket->addAction( topArtists );
    ui.artistsBucket->addAction( recentArtists );
    
    connect( topArtists, SIGNAL( toggled( bool)), SLOT( onTopArtistsToggled( bool )));
    connect( recentArtists, SIGNAL( toggled( bool)), SLOT( onRecentArtistsToggled( bool )));
#endif

    
    QWidget* freeInputWidget = new SpecialWidget;
    new QHBoxLayout( freeInputWidget );
    freeInputWidget->layout()->addWidget( ui.freeInput = new QLineEdit );
    ui.freeInput->setAttribute( Qt::WA_MacShowFocusRect, false );
    connect( ui.freeInput, SIGNAL( returnPressed()), SLOT( onFreeInputReturn()));
    
    freeInputWidget->layout()->addWidget( ui.inputSelector = new QComboBox );
    ui.inputSelector->addItem( tr( "Artist" ), QVariant::fromValue(Seed::ArtistType) );
    ui.inputSelector->addItem( tr( "Tag" ), QVariant::fromValue(Seed::TagType) );
    ui.inputSelector->addItem( tr( "User" ), QVariant::fromValue(Seed::UserType) );
    ui.inputSelector->setAttribute( Qt::WA_MacNormalSize ); //wtf? needed tho, when floating
    
    layout()->addWidget( freeInputWidget );
    
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
	UnicornWidget::paintItBlack( this );
    
    
}


void
Sources::onFreeInputReturn()
{
    if( ui.freeInput->text().trimmed().startsWith( "lastfm://" ))
    {
        RadioStation s( ui.freeInput->text() );
        The::app().open( s );
        ui.freeInput->clear();
        return;
    }
        
    
    if( !m_connectedAmp )
        return;
    
    Seed::Type type = ui.inputSelector->itemData(ui.inputSelector->currentIndex()).value<Seed::Type>();

    m_connectedAmp->addAndLoadItem( ui.freeInput->text(), type );
    
    ui.freeInput->clear();
}


void 
Sources::onUserGetFriendsReturn( WsReply* r )
{
    QList< User > users = User::list( r );
    
    foreach( User user, users )
    {
        PlayableListItem* n = new PlayableListItem( user, ui.friendsBucket );
        
        QNetworkReply* r = m_accessManager->get( QNetworkRequest( user.mediumImageUrl()));
        connect( r, SIGNAL( finished()), n, SLOT( iconDataDownloaded()));
        
        n->setPlayableType( Seed::UserType );	
    }
    ui.friendsBucket->reset();
}


void 
Sources::onUserGetTopTagsReturn( WsReply* r )
{
    WeightedStringList tags = Tag::list( r );
    if( tags.count() < 1 )
        return;
    
    QIcon tagIcon;
    tagIcon.addPixmap( QPixmap( ":/buckets/tag_39.png" ) );
    tagIcon.addPixmap( QPixmap( ":/buckets/tag_39.png" ), QIcon::Selected );
    tagIcon.addPixmap( QPixmap( ":/buckets/tag_21.png" ) );
    tagIcon.addPixmap( QPixmap( ":/buckets/tag_21.png" ), QIcon::Selected );
    foreach( WeightedString tag, tags )
    {
        PlayableListItem* n = new PlayableListItem( tag, ui.tagsBucket );
        n->setIcon( tagIcon );
        n->setPlayableType( Seed::TagType );
    }
}


void 
Sources::onUserGetArtistsReturn( WsReply* r )
{
    QList<Artist> artists = Artist::list( r );
    foreach( Artist a, artists )
    {
        PlayableListItem* n = new PlayableListItem( a, ui.artistsBucket );
        QNetworkReply* r = m_accessManager->get( QNetworkRequest( a.imageUrl()));
        connect( r, SIGNAL( finished()), n, SLOT( iconDataDownloaded()));
    }
}


void
Sources::onAuthUserInfoReturn( WsReply* r )
{
    QList<CoreDomElement> images = r->lfm().children( "image" );
    if( images.isEmpty() )
        return;
    
    QNetworkReply* reply = m_accessManager->get( QNetworkRequest( images.first().text() ));
    
    connect( reply, SIGNAL( finished()), SLOT( authUserIconDataDownloaded()));
}


void 
Sources::onUserGetPlaylistsReturn( WsReply* r )
{
Q_UNUSED( r )
#if 0 //FIXME: No RQL for playlists yet!
    QList<CoreDomElement> playlists = r->lfm().children( "playlist" );
    foreach( CoreDomElement playlist, playlists )
    {
        PlayableListItem* n = new PlayableListItem( playlist[ "title" ].text(), ui.stationsBucket );

        QString smallImageUrl = playlist.optional( "image size=small").text();
        if( !smallImageUrl.isEmpty() )
        {
            QNetworkReply* r = m_accessManager->get( QNetworkRequest( smallImageUrl));
            connect( r, SIGNAL( finished()), n, SLOT( iconDataDownloaded()));
        } 
        
        n->setPlayableType( Seed::PreDefinedType );
        //FIXME: this is not yet implemented in RQL
        n->setRQL( "" );
        
    }
#endif
}


void 
Sources::onItemDoubleClicked( const QModelIndex& index )
{
    if( !m_connectedAmp )
        return;
    
    SourcesList* itemView = dynamic_cast< SourcesList*>( sender() );
    Q_ASSERT( itemView );

    if( !(itemView->itemFromIndex( index )->flags() & Qt::ItemIsEnabled) )
        return;
    
    QStyleOptionViewItem options;
    options.initFrom( itemView );
    options.decorationSize = itemView->iconSize();
    options.displayAlignment = Qt::AlignVCenter | Qt::AlignLeft;
    options.decorationAlignment = Qt::AlignVCenter | Qt::AlignCenter;
    options.rect = itemView->visualRect( index );
    
    itemView->itemFromIndex( index )->setFlags( Qt::NoItemFlags );
    
    DelegateDragHint* w = new DelegateDragHint( itemView->itemDelegate( index ), index, options, itemView );
    w->setMimeData( itemView->mimeData( QList<QListWidgetItem*>()<< itemView->itemFromIndex(index) ) );
    w->dragToChild<QAbstractItemView*>( m_connectedAmp );
    connect( w, SIGNAL( finishedAnimation()), SLOT( onDnDAnimationFinished()));
}


void 
Sources::onDnDAnimationFinished()
{
    DelegateDragHint* delegateWidget = static_cast<DelegateDragHint*>(sender());
    QModelIndex index = delegateWidget->index();
}


void 
Sources::onAmpSeedRemoved( QString text, Seed::Type type )
{
    switch ( type ) {
        case Seed::UserType :
            foreach( QListWidgetItem* item, ui.friendsBucket->findItems( text, Qt::MatchFixedString ))
            {
                item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
            }
            break;
            
        case Seed::TagType:
            foreach( QListWidgetItem* item, ui.tagsBucket->findItems( text, Qt::MatchFixedString ))
            {
                item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
            }
            break;
            
        case Seed::PreDefinedType:
            foreach( QListWidgetItem* item, ui.stationsBucket->findItems( text, Qt::MatchFixedString ))
            {
                item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
            }
            break;            
        default:
            break;
    }
}


void 
Sources::connectToAmp( Amp* amp )
{
    if( m_connectedAmp )
        m_connectedAmp->disconnect( this );
    
    m_connectedAmp = amp;
    connect( m_connectedAmp, SIGNAL(itemRemoved(QString, Seed::Type)), SLOT( onAmpSeedRemoved(QString, Seed::Type)));
    connect( m_connectedAmp, SIGNAL(destroyed()), SLOT( onAmpDestroyed()));
}


void 
Sources::authUserIconDataDownloaded()
{
    QNetworkReply* reply = static_cast< QNetworkReply* >( sender() );

    QPixmap pm;
    if ( pm.loadFromData( reply->readAll()) ) 
    {
        QListWidgetItem* item;
        for( int i = 0; (item = ui.stationsBucket->item( i )); i++ )
        {
            PlayableListItem* pitem;
            if( !(pitem = dynamic_cast< PlayableListItem*>( item )))
                continue;
            
            pitem->setPixmap( pm );
        }
    }
}


void 
Sources::onContextMenuRequested( const QPoint& pos )
{
    m_cogMenu->move( pos );
    m_cogMenu->show();
}


void 
Sources::onCogMenuClicked()
{
    emit customContextMenuRequested( ui.cog->mapToGlobal( QPoint( ui.cog->geometry().width() - m_cogMenu->sizeHint().width(), ui.cog->height()) ) );
}


void 
Sources::onTabChanged()
{
    SourcesList* listView = qobject_cast< SourcesList* >( ui.tabWidget->currentWidget() );
    if( !listView )
        return;
    
    m_cogMenu->clear();
    
    foreach( QAction* a, listView->actions() )
    {
        m_cogMenu->addAction( a );
    }
       
}


void 
Sources::onTopArtistsToggled( bool b )
{
    if( !b )
        return;
    
    AuthenticatedUser authUser;
    ui.artistsBucket->clear();
    connect( authUser.getTopArtists(), SIGNAL(finished( WsReply* )), SLOT(onUserGetArtistsReturn( WsReply* )) );
    
}


void 
Sources::onRecentArtistsToggled( bool b )
{
    if( !b )
        return;
    
    AuthenticatedUser authUser;
    ui.artistsBucket->clear();
    connect( authUser.getRecentArtists(), SIGNAL(finished( WsReply* )), SLOT(onUserGetArtistsReturn( WsReply* )) );
}