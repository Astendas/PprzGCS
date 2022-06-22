#include "python_plugins.h"
#include "pprz_dispatcher.h"
#include "AircraftManager.h"
#include "pythonutils.h"

PythonPlugins::PythonPlugins(PprzApplication* app, PprzToolbox* toolbox) : PprzTool(app, toolbox)
{

}

void PythonPlugins::setToolbox(PprzToolbox* toolbox) {
    PprzTool::setToolbox(toolbox);
}

void PythonPlugins::bind_main_window(PprzMain* w) {
    if (!PythonUtils::bindAppObject("__main__", "mainWindow", PythonUtils::MainWindowType, w)) {
        qDebug() << "Error loading the application module";
    }
}
void PythonPlugins::bind_toolbox(PprzToolbox* t){
    if (!PythonUtils::bindAppObject("__main__", "toolbox", PythonUtils::PprzToolboxType, t)) {
        qDebug() << "Error loading the application module";
    }
}
void PythonPlugins::bind_app(PprzApplication* a){
    if (!PythonUtils::bindAppObject("__main__", "pprzApp", PythonUtils::PprzApplication, a)) {
        qDebug() << "Error loading the application module";
    }
}

void PythonPlugins::printDiagnostics()
{
    const QStringList script = QStringList()
            << "import sys" << "print('Path=', sys.path)" << "print('Executable=', sys.executable)";
    runScript(script);
}

void PythonPlugins::runScript(const QStringList &script)
{
    if (!::PythonUtils::runScript(script)) {
        qDebug() << "Error running script";
    }
}
void PythonPlugins::runThreadedScript(const QStringList &scripts)
{
    pythonThread* t1=new pythonThread();
    t1->setParent(this);
    t1->setScript(scripts);
    t1->start();
}
void pythonThread::run()
{
    if (!::PythonUtils::runScript(script)) {
        qDebug() << "Error running script";
    }
}

namespace PythonUtils {
bool __attribute__((weak))
runScript(const QStringList &script)
{return false;}

bool __attribute__((weak))
bindAppObject(const QString &moduleName, const QString &name, int index, QObject *o)
{return false;}

}

