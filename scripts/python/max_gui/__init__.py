##
#	\namespace	Q3dsMax
#
#	\remarks	[REMARKS]
#	
#	\author		Eric Hulser
#	\author		eric@blur.com
#	\author		Blur Studio
#	\date		07/07/08
#

from PyQt4.QtCore	import SIGNAL
from PyQt4.QtCore	import QTimer

from PyQt4.QtGui	import QApplication
from PyQt4.QtGui	import QDialog
from PyQt4.QtGui	import QMainWindow
from PyQt4.QtGui	import QWidget

import Py3dsMax
import win32gui
import pyHook

#-------------------------------------------------------------------------------------------------------------
# Basic 3dsMax Dialog Class

class Dialog( QDialog ):
	def __init__( self ):
		QDialog.__init__( self, application.window() )
	
	def accept( self ):
		application.closed( self )
		QDialog.accept( self )
	
	def enterEvent( self, event ):
		# Disabling the accelerators in 3dsMax allows keyboard entry for QDialogs
		Py3dsMax.mxs.enableAccelerators = False
		QDialog.enterEvent( self, event )
	
	def closeEvent( self, event ):
		application.closed( self )
		QDialog.closeEvent( self, event )
	
	def launch( self ):
		application.launch( self )
	
	def reject( self ):
		application.closed( self )
		QDialog.reject( self )

#-------------------------------------------------------------------------------------------------------------

class Window( QMainWindow ):
	def __init__( self ):
		QMainWindow.__init__( self, application.window() )
		
	def enterEvent( self, event ):
		# Disabling the accelerators in 3dsMax allows keyboard entry for QDialogs
		Py3dsMax.mxs.enableAccelerators = False
		QMainWindow.enterEvent( self, event )
	
	def closeEvent( self, event ):
		QMainWindow.closeEvent( self, event )
		application.closed( self )
	
	def launch( self ):
		application.launch( self )

#-------------------------------------------------------------------------------------------------------------

class MaxApplication( QApplication ):
	def __init__( self ):
		QApplication.__init__( self, [] )
		
		# Build Hook Manager
		self._running 				= False
		self._hookManager			= pyHook.HookManager()
		self._hookManager.KeyAll	= self.hookEvent
		self._hookManager.MouseAll	= self.hookEvent
		
		# Build Timer
		self._timer					= QTimer()
		self._timer.setInterval( 350 )
		self.connect( self._timer, SIGNAL( 'timeout()' ), self.timeout )
		
		# Build Window
		self._window = QWidget()
		self._window.setWindowTitle( '3dsMaxWidget' )
		handle 						= win32gui.FindWindow( 0, '3dsMaxWidget' )
		left, right, width, height 	= win32gui.GetWindowRect( Py3dsMax.GetWindowHandle() )
		win32gui.SetParent( handle, Py3dsMax.GetWindowHandle() )
		
		self._window.move( left, right )
		self._window.resize( width, height )
		
		self._dialogs = []
	
	def closed( self, dialog ):
		if ( dialog in self._dialogs ):
			self._dialogs.remove( dialog )
			if ( not self._dialogs ):
				self.kill()
	
	def kill( self ):
		if ( self.isRunning() ):
			Py3dsMax.mxs.messageBox( 'Unhooking Events!' )
			self._hookManager.UnhookKeyboard()
			self._hookManager.UnhookMouse()
			self._running = False
			self._timer.stop()
	
	def launch( self, dialog ):
		if ( not dialog in self._dialogs ):
			self._dialogs.append( dialog )
			dialog.show()
			self.run()
		return True
	
	def isRunning( self ):
		return self._running
	
	def hookEvent( self, event ):
		self.sendPostedEvents( None, -1 )
		return True
	
	def run( self ):
		if ( not self.isRunning() ):
			Py3dsMax.mxs.messageBox( 'Hooking Events!' )
			
			self._hookManager.HookKeyboard()
			self._hookManager.HookMouse()
			self._running = True
			
			self._timer.start()
	
	def timeout( self ):
		self.sendPostedEvents( None, -1 )
		self._timer.start()
	
	def window( self ):
		return self._window

#-------------------------------------------------------------------------------------------------------------
application		= None
logger			= None

# Initialize QApplication
if ( not QApplication.instance() ):
	application 	= MaxApplication()
else:
	application		= QApplication.instance()
	
#-------------------------------------------------------------------------------------------------------------