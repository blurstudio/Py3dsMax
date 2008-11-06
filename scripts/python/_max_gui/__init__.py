##
#	\namespace	max_gui::__init__
#
#	\remarks	Initializes a Qt Application to allow for QDialogs & QMainWindows to run inside of a 3dsMax session
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
	def __init__( self, *args ):
		if ( not args ):
			args = [core]
			
		QDialog.__init__( self, *args )
	
	def accept( self ):
		app.closed( self )
		QDialog.accept( self )
	
	def enterEvent( self, event ):
		# Disabling the accelerators in 3dsMax allows keyboard entry for QDialogs
		Py3dsMax.mxs.enableAccelerators = False
		QDialog.enterEvent( self, event )
	
	def closeEvent( self, event ):
		app.closed( self )
		QDialog.closeEvent( self, event )
	
	def launch( self ):
		app.launch( self )
	
	def reject( self ):
		app.closed( self )
		QDialog.reject( self )

#-------------------------------------------------------------------------------------------------------------

class Window( QMainWindow ):
	def __init__( self, *args ):
		if ( not args ):
			args = [core]
			
		QMainWindow.__init__( self, *args )
		
	def enterEvent( self, event ):
		# Disabling the accelerators in 3dsMax allows keyboard entry for QDialogs
		Py3dsMax.mxs.enableAccelerators = False
		QMainWindow.enterEvent( self, event )
	
	def closeEvent( self, event ):
		QMainWindow.closeEvent( self, event )
		app.closed( self )
	
	def launch( self ):
		app.launch( self )

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
		
		self._dialogs = []
	
	def closed( self, dialog ):
		if ( dialog in self._dialogs ):
			self._dialogs.remove( dialog )
			if ( not self._dialogs ):
				self.kill()
	
	def kill( self ):
		if ( self.isRunning() ):
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
			self._hookManager.HookKeyboard()
			self._hookManager.HookMouse()
			self._running = True
			
			self._timer.start()
	
	def timeout( self ):
		self.sendPostedEvents( None, -1 )
		self._timer.start()

#-------------------------------------------------------------------------------------------------------------
# Build Gui

app				= None

# Initialize QApplication
if ( not QApplication.instance() ):
	app 	= MaxApplication()
	app.setStyle( 'Plastique' )
else:
	app		= QApplication.instance()

# Build Core
core = QWidget()
core.setWindowTitle( '3dsMaxWidget' )

handle 						= win32gui.FindWindow( 0, '3dsMaxWidget' )
left, right, width, height 	= win32gui.GetWindowRect( Py3dsMax.GetWindowHandle() )
win32gui.SetParent( handle, Py3dsMax.GetWindowHandle() )

core.move( left, right )
core.resize( width, height )
	
#-------------------------------------------------------------------------------------------------------------
# Create Default Gui Dialogs
logger				= None
progress			= None

def init():
	# Create Logger Window
	global logger
	from LoggerWindow 		import LoggerWindow
	logger					= LoggerWindow()
	logger.launch()
	
	# Create Progress Window
	global progress
	from ProgressDialog		import ProgressDialog
	progress				= ProgressDialog()