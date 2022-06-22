#pragma once

#include <QObject>
#include "../PprzToolbox.h"
#include "../PprzApplication.h"
//#include "pprz_dispatcher.h"
#include "../pprzmain.h"

class PythonPlugins : public PprzTool
{
    Q_OBJECT
public:
    explicit PythonPlugins(PprzApplication* app, PprzToolbox* toolbox);
    void setToolbox(PprzToolbox* toolbox) override;

    void bind_main_window(PprzMain* w);
    void bind_toolbox(PprzToolbox* t);
    void bind_app(PprzApplication* a);

signals:

public Q_SLOTS:
    void printDiagnostics();
    void runScript(const QStringList &);
    void runThreadedScript(const QStringList &);
};
class pythonThread: public QThread
{
    Q_OBJECT
public:
    void run();
    void setScript(const QStringList &scripts){script=scripts;};
private:
    QStringList script;
};
