##
#	\namespace	ProgressDialog
#
#	\remarks	[ADD REMARKS]
#	
#	\author		Eric Hulser
#	\author		Email: eric@blur.com
#	\author		Company: Blur Studio
#	\date		01/20/08
#

import sys

import PyQt4.uic
from PyQt4.QtCore		import SIGNAL
from PyQt4.QtGui		import QPalette

from _max_gui		import Dialog

import Py3dsMax

#-------------------------------------------------------------------------------------------------------------

class ProgressSection:
	def __init__( self, parent, count ):
		self._count 	= count
		self._parent	= parent
		self._index		= 0
	
	def increment( self ):
		self._index += 1
	
	def index( self ):
		return self._index
	
	def parent( self ):
		return self._parent
	
	def percent( self, recursive = True ):
		outPercent = 1.0
		
		if ( self._count ):
			outPercent /= self._count
		
		if ( recursive ):
			
			sect = self.parent()
			while ( sect ):
				outPercent *= sect.percent()
				sect		= sect.parent()
		
		return outPercent
	
	def value( self, recursive = True ):
		if ( recursive ):
			outValue		= 0
			
			sect 			= self._parent
			while ( sect ):
				outValue	+= 100 * ( sect.index() * sect.percent() )
				sect		= sect.parent()
			
			return outValue + ( 100 * self.index() * self.percent() )
		else:
			return ( 100 * self.index() * self.percent( recursive = False ) )

class ProgressDialog( Dialog ):
	def __init__( self, *args ):
		Dialog.__init__( self, *args )
		PyQt4.uic.loadUi( Py3dsMax.installPath + '_max_gui/resource/ProgressDialog.ui', self )
		
		self._canceled 		= False
		self._sections		= []
		
		self.connect( self.uiCancelBTN, SIGNAL( 'clicked()' ), self.setCanceled )
	
	def canceled( self ):
		return self._canceled
		
	def closeEvent( self, event ):
		self._sections = []
		Dialog.closeEvent( self, event )
	
	def count( self ):
		if ( self._sections ):
			return self._sections[0].count()
		return 0
	
	def increment( self, amount = 1 ):
		success = False
		
		if ( self._sections ):
			sect = self._sections[-1]
			sect.increment()
			success = self.update()
		
		return success
	
	def setCanceled( self, state = True ):
		self._canceled = state
	
	def setCount( self, count ):
		self._sections = [ ProgressSection( None, count ) ]
		return self.update()
	
	def setText( self, text ):
		self.uiCaptionLBL.setText( text )
		self.uiCaptionLBL.setVisible( text != '' )
		#self.repaint()
	
	def setValue( self, value ):
		self.uiProgressPBR.setValue( value )
	
	def start( self, caption, count, canCancel = False, showSecondary = False ):
		if ( not self._sections ):
			palette = self.uiProgressPBR.palette()
			self.uiProgressPBR.setPalette( palette )
			self.uiSecondaryPBR.setPalette( palette )
			
			self.uiCancelBTN.setVisible( canCancel )
			self.uiSecondaryPBR.setVisible( showSecondary )
			
			self.setText( '' )
			self.setCount( count )
			self.setValue( 0 )
			self.show()
			self.setWindowTitle( caption )
		else:
			self._sections.append( ProgressSection( self._sections[-1], count ) )
			self.setText( caption )
	
	def stop( self ):
		# Pop the top section
		if ( self._sections ):
			self._sections.pop()
		
		# if there are no more sections, close out
		if ( not self._sections ):
			self.close()
	
	def update( self ):
		success = False
		if ( self._sections ):
			sect = self._sections[-1]
			self.setValue( sect.value() )
			
			if ( self.uiSecondaryPBR.isVisible() ):
				self.uiSecondaryPBR.setValue( sect.value( recursive = False ) )
				
			success = True
		return success
		
	def value( self ):
		return self.uiProgressPBR.value()