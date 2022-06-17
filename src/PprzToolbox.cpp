#include "PprzToolbox.h"
#include "AircraftManager.h"
#include "srtm_manager.h"
#include "pprz_dispatcher.h"
#include "coordinatestransform.h"
#include "units.h"
#include "dispatcher_ui.h"
#include "watcher.h"
#include "python_plugins.h"

#if defined(ADAPTIVE_ENABLED)
#include "networkAdapter.h"
#endif

#if defined(SPEECH_ENABLED)
#include "speaker.h"
#endif

PprzToolbox::PprzToolbox(PprzApplication* app)
{
    _aircraftManager      = new AircraftManager     (app, this);
    _srtmManager          = new SRTMManager         (app, this);
    _pprzDispatcher       = new PprzDispatcher      (app, this);
    _coordinatesTransform = new CoordinatesTransform(app, this);
    _units                = new Units               (app, this);
    _dispatcherUi         = new DispatcherUi        (app, this);
    _watcher              = new Watcher             (app, this);
    _plugins              = new PythonPlugins       (app, this);
#if defined(SPEECH_ENABLED)
    _speaker              = new Speaker             (app, this);
#endif

#if defined(ADAPTIVE_ENABLED)
    _networkAdapter       = new NetworkAdapter      (app, this);
#endif
}

void PprzToolbox::setChildToolboxes(void) {
    _aircraftManager->setToolbox(this);
    _srtmManager->setToolbox(this);
    _pprzDispatcher->setToolbox(this);
    _coordinatesTransform->setToolbox(this);
    _units->setToolbox(this);
    _dispatcherUi->setToolbox(this);
    _watcher->setToolbox(this);
    _plugins->setToolbox(this);
#if defined(SPEECH_ENABLED)
    _speaker->setToolbox(this);
#endif
#if defined(ADAPTIVE_ENABLED)
    if(adaptive){
        _networkAdapter->setToolbox(this);
    }
#endif
}


PprzTool::PprzTool(PprzApplication* app, PprzToolbox* toolbox) :
    QObject(toolbox),
    _app(app), _toolbox(nullptr)
{
    (void)toolbox;
}

void PprzTool::setToolbox(PprzToolbox* toolbox)
{
    _toolbox = toolbox;
}
