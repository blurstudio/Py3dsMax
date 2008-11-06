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
import string
import inspect

import PyQt4.uic
from PyQt4.QtCore		import Qt
from PyQt4.QtCore		import QTimer
from PyQt4.QtCore		import QPoint
from PyQt4.QtCore		import SIGNAL

from PyQt4.QtGui		import QBrush
from PyQt4.QtGui		import QColor
from PyQt4.QtGui		import QCursor
from PyQt4.QtGui		import QDialog
from PyQt4.QtGui		import QHBoxLayout
from PyQt4.QtGui		import QListWidget
from PyQt4.QtGui		import QPalette
from PyQt4.QtGui		import QTextCursor
from PyQt4.QtGui		import QTextEdit
from PyQt4.QtGui		import QTextCharFormat
from PyQt4.QtGui		import QToolTip

import Py3dsMax
import _max_gui

from Py3dsMax			import mxs
from _max_gui			import Window

#-------------------------------------------------------------------------------------------------------------

class InspectorDialog( QDialog ):
	def __init__( self, editor, keys ):
		QDialog.__init__( self, editor )
		self.setWindowTitle( 'Auto-Complete' )
		self._editor = editor
		
		keys.sort()
		self._listWidget 	= QListWidget( self )
		self._listWidget.addItems( keys )
		self._listWidget.setTabKeyNavigation( False )
		self._listWidget.keyPressEvent = self.handleKeyPress
		self._listWidget.focusOutEvent = self.handleFocusOut
		
		layout 				= QHBoxLayout()
		layout.addWidget( self._listWidget )
		layout.setMargin( 0 )
		layout.setSpacing( 0 )
		self.setLayout( layout )
		
		self._listWidget.setFocus()
	
	def handleFocusOut( self, event ):
		self.reject()
	
	def handleKeyPress( self, event ):
		# If tab is pressed, then set the value to the inputed item
		if ( event.key() == Qt.Key_Tab ):
			self.reject()
			
			# Clean out the current typed code
			code 		= str( self._editor.textCursor().block().text() ).split()[-1]
			while ( code[-1] != '.' ):
				self._editor.textCursor().deletePreviousChar()
				code = code[:-1]
			
			self._editor.insertPlainText( self._listWidget.currentItem().text() )
		
		# Kill for spaces, periods and returns - pass along to editor
		elif ( event.key() in ( Qt.Key_Space, Qt.Key_Period, Qt.Key_Enter, Qt.Key_Return, Qt.Key_Escape, Qt.Key_ParenLeft, Qt.Key_Delete, Qt.Key_Backspace ) ):
			self.reject()
			self._editor.keyPressEvent( event )
		
		# Pass along up/down keys to the list widget
		elif ( event.key() == Qt.Key_Up or event.key() == Qt.Key_Down ):
			QListWidget.keyPressEvent( self._listWidget, event )
		
		# Otherwise, look up entry by code
		else:
			self._editor.keyPressEvent( event )
			
			# Look up by code
			code 		= str( self._editor.textCursor().block().text() ).split()[-1].split('.')[-1]
			if ( code ):
				items = self._listWidget.findItems( code, Qt.MatchStartsWith | Qt.MatchCaseSensitive )
				if ( items ):
					self._listWidget.setCurrentItem( items[0] )
				else:
					self._listWidget.clearSelection()

class ConsoleWidget( QTextEdit ):
	def __init__( self, parent ):
		QTextEdit.__init__( self, parent )
		self.startInputLine()
		self._timer = QTimer()
		QTextEdit.connect( self._timer, SIGNAL( 'timeout()' ), self.timeout )
		
	def clear( self ):
		QTextEdit.clear( self )
		self.startInputLine()
	
	def executeCommand( self ):
		textblock 	= self.textCursor().block().text()
		results 	= re.search( '>>> (.*)', str(textblock) )
		
		if ( results ):
			if ( self.textCursor().atEnd() ):
				self.insertPlainText( '\n' )
				
				command 	= str( results.groups()[0] )
				cmdresult 	= None
				
				try:
					cmdresult = eval( command, globals() )
				except:
					exec ( command ) in globals()
				
				if ( cmdresult != None ):
					print cmdresult
					
				self.startInputLine()
			else:
				self.startInputLine()
				self.insertPlainText( str( results.groups()[0] ) )
		else:
			self.startInputLine()
		
	def keyPressEvent( self, event ):
		if ( event.key() == Qt.Key_Return or event.key() == Qt.Key_Enter ):
			self.executeCommand()
		elif ( event.key() == Qt.Key_Home ):
			mode = QTextCursor.MoveAnchor
			
			# Select Home
			if ( event.modifiers() == Qt.ShiftModifier ):
				mode = QTextCursor.KeepAnchor
			
			cursor 	= self.textCursor()
			block	= str( cursor.block().text() ).split()
			cursor.movePosition( QTextCursor.StartOfBlock, mode )
			cursor.movePosition( QTextCursor.Right, mode, 4 )
			self.setTextCursor( cursor )
			
		else:
			QTextEdit.keyPressEvent( self, event )
			
			# Check for hints (doc hints and attribute hints)
			if ( event.key() == Qt.Key_Period or event.key() == Qt.Key_ParenLeft ):
				# Look up data for hint
				data		= None
				try:
					data = eval( str( self.textCursor().block().text() ).split()[-1][:-1] )
				except:
					pass
				
				if ( data ):
					point		= self.mapToGlobal( QPoint( self.cursorRect().x(), self.cursorRect().y() + 18) )
					
					# Look up attributes
					if ( event.key() == Qt.Key_Period ):
						if ( _max_gui.logger.isHintingEnabled() ):
							keys = [ name for name, value in inspect.getmembers( data ) if not name.startswith( '_' ) ] # Collect non-hidden method/variable names
							
							if ( keys ):
								self.timeout()
								dialog	= InspectorDialog( self, keys )
								dialog.move( point.x(), point.y() )
								dialog.show()
					
					# Look up docs
					elif ( event.key() == Qt.Key_ParenLeft ):
						tip = inspect.getdoc( data )
						if ( tip == None ):
							tip = ''
							
						if ( inspect.isfunction( data ) or ( inspect.ismethod( data ) and not str( type( data ) ) == "<type 'instancemethod'>" ) ):
							argspecs 	= inspect.getargspec( data )
							args		= argspecs[0]
							defaults	= argspecs[3]
							
							# Build Defaults
							if ( defaults ):
								index		= len( args ) - len( defaults )
								for default in defaults:
									default = str( default )
									if ( default == '' ):
										default = "''"
										
									args[index] = args[index] + ' = ' + default
									index += 1
							
							# Remove 'self' from initial list
							if ( args and args[0] == 'self' ):
								args = args[1:]
							
							tip = '(' + string.join( args, ',' ) + ')\n' + tip
						
						tip = string.join( [ part.strip() for part in tip.split('\n') if part != '' ], '\n' )
						
						if ( tip ):
							QToolTip.showText( point, tip )
							self._timer.start(5000)
			
			# Hide data docs
			elif ( event.key() == Qt.Key_ParenRight or event.key() == Qt.Key_Escape ):
					self.timeout()
	
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
	
	def timeout( self ):
		self._timer.stop()
		QToolTip.hideText()
	
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
		_max_gui.logger.flush()
	
	def write( self, msg ):
		_max_gui.logger.write( msg, error = True )

class LoggerWindow( Window ):
	def __init__( self ):
		Window.__init__( self )
		PyQt4.uic.loadUi( Py3dsMax.installPath + '_max_gui/resource/LoggerWindow.ui', self )
		
		self._console = ConsoleWidget( self )
		self._console.palette().setColor( QPalette.Background, QColor( 220, 220, 220 ) )
		self._console.palette().setColor( QPalette.Foreground, QColor( 0, 0, 0 ) )
		
		self._hintingEnabled = True
		self.setHintingEnabled( self._hintingEnabled )
		
		layout = QHBoxLayout()
		layout.addWidget( self._console )
		self.centralWidget().setLayout( layout )
		
		# Hijack stdout & stderr to run through the logger
		sys.stdout			= self
		sys.stderr			= ErrLog()
		
		self._active = True
		self._connect()
	
	def _connect( self ):
		self.connect( self.uiClearLogACT,		SIGNAL( 'triggered()' ),		self.flush )
		self.connect( self.uiSaveLogACT,		SIGNAL( 'triggered()' ),		self.saveLog )
#		self.connect( self.uiNewScriptACT,		SIGNAL( 'triggered()' ),		trax.gui.core.newScript )
#		self.connect( self.uiOpenScriptACT,		SIGNAL( 'triggered()' ),		trax.gui.core.openScript )
#		self.connect( self.uiRunScriptACT,		SIGNAL( 'triggered()' ),		trax.gui.runScript )
		self.connect( self.uiHintingEnabledACT,	SIGNAL( 'triggered()' ),		self.toggleHintingEnabled )
	
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
	
	def isHintingEnabled( self ):
		return self._hintingEnabled
		
	def flush( self ):
		self._console.clear()
	
	def write( self, msg, error = False ):
		if ( self.isActive() ):
			self._console.write( msg, error = error )
	
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
	
	def setHintingEnabled( self, state ):
		self._hintingEnabled = state
		self.uiHintingEnabledACT.setChecked( state )
	
	def toggle( self ):
		self.setVisible( not self.isVisible() )
	
	def toggleHintingEnabled( self ):
		self.setHintingEnabled( not self.isHintingEnabled() )