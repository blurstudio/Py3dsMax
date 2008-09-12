##
#	\namespace	LoggerWindow
#
#	\remarks	[ADD REMARKS]
#	
#	\author		Eric Hulser
#	\author		Email: eric@blur.com
#	\author		Company: Blur Studio
#	\date		01/15/08
#

import re
import sys

import PyQt4.uic
from PyQt4.QtCore		import Qt
from PyQt4.QtCore		import SIGNAL

from PyQt4.QtGui		import QBrush
from PyQt4.QtGui		import QColor
from PyQt4.QtGui		import QHBoxLayout
from PyQt4.QtGui		import QPalette
from PyQt4.QtGui		import QTextCursor
from PyQt4.QtGui		import QTextEdit
from PyQt4.QtGui		import QTextCharFormat

import max_gui

from Py3dsMax			import mxs
from max_gui			import Window

#-------------------------------------------------------------------------------------------------------------

class ConsoleWidget( QTextEdit ):
	def __init__( self, parent ):
		QTextEdit.__init__( self, parent )
		self.startInputLine()
		
	def clear( self ):
		QTextEdit.clear( self )
		self.startInputLine()
	
	def executeCommand( self ):
		textblock 	= self.textCursor().block().text()
		results 	= re.search( '>>> (.*)', str(textblock) )
		
		if ( results ):
			if ( self.textCursor().atEnd() ):
				self.insertPlainText( '\n' )
				
				exec ( str( results.groups()[0] ) ) in globals()	
				self.startInputLine()
			else:
				self.startInputLine()
				self.insertPlainText( str( results.groups()[0] ) )
		else:
			self.startInputLine()
	
	def enterEvent( self, event ):
		mxs.enableAccelerators = False
		QTextEdit.enterEvent( self, event )
		
	def keyPressEvent( self, event ):
		if ( event.key() == Qt.Key_Return or event.key() == Qt.Key_Enter ):
			self.executeCommand()
		else:
			QTextEdit.keyPressEvent( self, event )
	
	def startInputLine( self ):
		self.moveCursor( QTextCursor.End )
		
		if ( self.textCursor().block().text() != '>>> ' ):
			charFormat = QTextCharFormat()
			charFormat.setForeground( QColor( 0, 0, 0 ) )
			self.setCurrentCharFormat( charFormat )
			
			inputstr = '>>> '
			if ( str(self.textCursor().block().text()) ):
				inputstr = '\n' + inputstr
				
			self.insertPlainText( inputstr )
	
	def write( self, msg, error = False ):
		self.moveCursor( QTextCursor.End )
		
		charFormat = QTextCharFormat()
		
		if ( not error ):
			charFormat.setForeground( QColor( 0, 0, 255 ) )
		else:
			charFormat.setForeground( QColor( 193, 0, 0 ) )
			
		self.setCurrentCharFormat( charFormat )
		
		self.insertPlainText( msg )

class ErrLog:
	def flush( self ):
		max_gui.logger.flush()
	
	def write( self, msg ):
		max_gui.logger.write( msg, error = True )

class LoggerWindow( Window ):
	def __init__( self, **args ):
		Window.__init__( self )
		
		PyQt4.uic.loadUi( Py3dsMax.installPath + 'max_gui/resource/LoggerWindow.ui', self )
		
		self._console = ConsoleWidget( self )
		
		self._console.palette().setColor( QPalette.Background, QColor( 220, 220, 220 ) )
		self._console.palette().setColor( QPalette.Foreground, QColor( 0, 0, 0 ) )
		
		layout = QHBoxLayout()
		layout.addWidget( self._console )
		self.centralWidget().setLayout( layout )
		
		# Hijack stdout & stderr to run through the logger
		self._orig_stdout	= sys.stdout
		self._orig_stderr	= sys.stderr
		
		sys.stdout			= self
		sys.stderr			= ErrLog()
		
		self._active = True
		self._connect()
	
	def _connect( self ):
		#self.connect( self.uiClearLogACT,		SIGNAL( 'triggered()' ),		self.flush )
		#self.connect( self.uiSaveLogACT,		SIGNAL( 'triggered()' ),		self.saveLog )
		#self.connect( self.uiNewScriptACT,		SIGNAL( 'triggered()' ),		trax.gui.core.newScript )
		#self.connect( self.uiOpenScriptACT,		SIGNAL( 'triggered()' ),		trax.gui.core.openScript )
		#self.connect( self.uiRunScriptACT,		SIGNAL( 'triggered()' ),		trax.gui.core.runScript )
		pass
	
	#-------------------------------------------------------------------------------------------------------------
	
	def activate( self ):
		if ( not self.isActive() ):
			self.setActive( True )
	
	def consoleWidget( self ):
		return self._console
	
	def deactivate( self ):
		if ( self.isActive() ):
			self.setActive( False )
	
	def isActive( self ):
		return self._active
		
	def flush( self ):
		self._console.clear()
		self._orig_stdout.flush()
		self._orig_stderr.flush()
	
	def write( self, msg, error = False ):
		if ( self.isActive() ):
			self._console.write( msg, error = error )
		
		if ( not error ):
			self._orig_stdout.write( msg )
		else:
			self._orig_stderr.write( msg )
	
	def saveLog( self ):
		fileName = mxs.getSaveFileName()
		if ( fileName ):
			f = open( fileName, 'w' )
			f.write( self._console.toPlainText() )
			f.close()
			return True
		return False
	
	def setActive( self, state ):
		self._active = state
	
	def toggle( self ):
		self.setVisible( not self.isVisible() )
		self.updateFlags()


if ( not max_gui.logger ):
	max_gui.logger = LoggerWindow()
max_gui.logger.launch()