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

#include "RadioMiniControls.h"
#include "PlayerState.h"
#include "ObservedTrack.h"
#include <phonon/volumeslider.h>


RadioMiniControls::RadioMiniControls()
{
    ui.setupUi( this );
    ui.play->setCheckedIcon( QIcon( ":/stop.png" ) );
    ui.volume = new Phonon::VolumeSlider;
	ui.volume->setMinimumWidth( ui.play->width() + ui.skip->width() );
	
    layout()->addWidget( ui.volume );
	
	connect( qApp, SIGNAL(stateChanged( State, Track )), SLOT(onStateChanged( State, Track )) );
	connect( ui.play, SIGNAL( clicked()), SLOT( onPlayClicked()) );
    
    onStateChanged( Stopped, Track() );
}


void 
RadioMiniControls::onStateChanged( State state, const Track& t )
{
	switch (state) 
	{
		case Playing:
            if (t.source() != Track::LastFmRadio)
                return;
        case TuningIn:
            ui.play->show();
            ui.skip->show();
            ui.volume->show();            
            ui.play->setChecked( true );
			break;
		
		case Stopped:
            ui.play->hide();
            ui.skip->hide();
            ui.volume->hide();            
            ui.play->setChecked( false );
			break;
            
        default:
            break;
	}
}


void
RadioMiniControls::onPlayClicked()
{
	if (!ui.play->isChecked())
		emit stop();
}
