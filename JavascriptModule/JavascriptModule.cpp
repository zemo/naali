/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   JavascriptModule.cpp
 *  @brief  Enables Javascript execution and scripting in Naali.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "JavascriptModule.h"
#include "ScriptMetaTypeDefines.h"
#include "JavascriptEngine.h"

#include "EC_Script.h"
#include "SceneManager.h"
#include "Input.h"
#include "UiServiceInterface.h"
#include "ISoundService.h"
#include "Frame.h"
#include "Console.h"
#include "ConsoleCommandServiceInterface.h"
#include "NaaliCoreTypeDefines.h"

#include <QtScript>

Q_SCRIPT_DECLARE_QMETAOBJECT(QPushButton, QWidget*) ///@todo Remove? This is already done in ScriptMetaTypeDefines.cpp

#include "MemoryLeakCheck.h"

std::string JavascriptModule::type_name_static_ = "Javascript";
JavascriptModule *javascriptModuleInstance_ = 0;

JavascriptModule::JavascriptModule() :
    IModule(type_name_static_),
    engine(new QScriptEngine(this))
{
}

JavascriptModule::~JavascriptModule()
{
}

void JavascriptModule::Load()
{
    DECLARE_MODULE_EC(EC_Script);
}


void JavascriptModule::PreInitialize()
{
}

void JavascriptModule::Initialize()
{
    connect(GetFramework(), SIGNAL(SceneAdded(const QString&)), this, SLOT(SceneAdded(const QString&)));

    LogInfo("Module " + Name() + " initializing...");

    assert(!javascriptModuleInstance_);
    javascriptModuleInstance_ = this;

    // Register ourselves as javascript scripting service.
    boost::shared_ptr<JavascriptModule> jsmodule = framework_->GetModuleManager()->GetModule<JavascriptModule>().lock();
    boost::weak_ptr<ScriptServiceInterface> service = boost::dynamic_pointer_cast<ScriptServiceInterface>(jsmodule);
    framework_->GetServiceManager()->RegisterService(Service::ST_JavascriptScripting, service);

    engine->globalObject().setProperty("print", engine->newFunction(Print));

    QScriptValue objectbutton= engine->scriptValueFromQMetaObject<QPushButton>();
    engine->globalObject().setProperty("QPushButton", objectbutton);

    JavascriptModule::RunScript("jsmodules/lib/json2.js");

    RunString("print('Hello from qtscript');");
}

void JavascriptModule::PostInitialize()
{
    RegisterNaaliCoreMetaTypes();

    // Add Naali Core API objcects as js services.
    services_["input"] = GetFramework()->GetInput();
    services_["ui"] = GetFramework()->UiService();
    services_["audio"] = GetFramework()->Audio();
    services_["frame"] = GetFramework()->GetFrame();
    services_["console"] = GetFramework()->Console();

    RegisterConsoleCommand(Console::CreateCommand(
        "JsExec", "Execute given code in the embedded Javascript interpreter. Usage: JsExec(mycodestring)", 
        Console::Bind(this, &JavascriptModule::ConsoleRunString)));

    RegisterConsoleCommand(Console::CreateCommand(
        "JsLoad", "Execute a javascript file. JsLoad(myjsfile.js)",
        Console::Bind(this, &JavascriptModule::ConsoleRunFile)));
    
    RegisterConsoleCommand(Console::CreateCommand(
        "JsReloadScripts", "Reloads and re-executes startup scripts.",
        Console::Bind(this, &JavascriptModule::ConsoleReloadScripts)));
    
    // Initialize startup scripts
    LoadStartupScripts();
}

void JavascriptModule::Uninitialize()
{
    UnloadStartupScripts();
}

void JavascriptModule::Update(f64 frametime)
{
    RESETPROFILER;
}

bool JavascriptModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    return false;
}

Console::CommandResult JavascriptModule::ConsoleRunString(const StringVector &params)
{
    if (params.size() != 1)
        return Console::ResultFailure("Usage: JsExec(print 1 + 1)");

    JavascriptModule::RunString(QString::fromStdString(params[0]));
    return Console::ResultSuccess();
}

Console::CommandResult JavascriptModule::ConsoleRunFile(const StringVector &params)
{
    if (params.size() != 1)
        return Console::ResultFailure("Usage: JsLoad(myfile.js)");

    JavascriptModule::RunScript(QString::fromStdString(params[0]));

    return Console::ResultSuccess();
}

Console::CommandResult JavascriptModule::ConsoleReloadScripts(const StringVector &params)
{
    LoadStartupScripts();

    return Console::ResultSuccess();
}

JavascriptModule *JavascriptModule::GetInstance()
{
    assert(javascriptModuleInstance_);
    return javascriptModuleInstance_;
}

void JavascriptModule::RunString(const QString &codestr, const QVariantMap &context)
{
    QMapIterator<QString, QVariant> i(context);
    while (i.hasNext())
    {
        i.next();
        engine->globalObject().setProperty(i.key(), engine->newQObject(i.value().value<QObject*>()));
    }

    engine->evaluate(codestr);
}

void JavascriptModule::RunScript(const QString &scriptFileName)
{
    QFile scriptFile(scriptFileName);
    scriptFile.open(QIODevice::ReadOnly);
    engine->evaluate(scriptFile.readAll(), scriptFileName);
    scriptFile.close();
}

void JavascriptModule::SceneAdded(const QString &name)
{
    Scene::ScenePtr scene = GetFramework()->GetScene(name);
    connect(scene.get(), SIGNAL(ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)),
            SLOT(ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)));
    connect(scene.get(), SIGNAL(ComponentRemoved(Scene::Entity*, IComponent*, AttributeChange::Type)),
            SLOT(ComponentRemoved(Scene::Entity*, IComponent*, AttributeChange::Type)));
}

void JavascriptModule::ScriptChanged(const QString &scriptRef)
{
    EC_Script *sender = dynamic_cast<EC_Script*>(this->sender());
    if(!sender)
        return;
    
    if(!scriptRef.endsWith(".js"))
    {
        // If script ref is empty or otherwise invalid we need to destroy the previous script if it's type is javascript.
        if(dynamic_cast<JavascriptEngine*>(sender->GetScriptInstance()))
        {
            JavascriptEngine *javaScriptInstance = new JavascriptEngine("");
            sender->SetScriptInstance(javaScriptInstance);
        }
        return;
    }
    
    if (sender->type.Get() != "js")
        return;
    
    JavascriptEngine *javaScriptInstance = new JavascriptEngine(scriptRef);
    sender->SetScriptInstance(javaScriptInstance);

    //Register all services to script engine
    PrepareScriptEngine(javaScriptInstance);

    //Send entity that owns the EC_Script component.
    javaScriptInstance->RegisterService(sender->GetParentEntity(), "me");
    //Send the scene that owns the script component.
    javaScriptInstance->RegisterService(sender->GetParentEntity()->GetScene(), "scene");

    if (sender->runOnLoad.Get())
        sender->Run();
}

void JavascriptModule::ComponentAdded(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if(comp->TypeName() == "EC_Script")
        connect(comp, SIGNAL(ScriptRefChanged(const QString &)), SLOT(ScriptChanged(const QString &)));
}

void JavascriptModule::ComponentRemoved(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if(comp->TypeName() == "EC_Script")
        disconnect(comp, SIGNAL(ScriptRefChanged(const QString &)), this, SLOT(ScriptChanged(const QString &)));
}

void JavascriptModule::LoadStartupScripts()
{
    UnloadStartupScripts();
    
    std::string path = "./jsmodules/startup";
    std::vector<std::string> scripts;
    
    try
    {
        boost::filesystem::directory_iterator i(path);
        boost::filesystem::directory_iterator end_iter;
        while (i != end_iter)
        {
            if (boost::filesystem::is_regular_file(i->status()))
            {
                std::string ext = i->path().extension();
                boost::algorithm::to_lower(ext);
                if (ext == ".js")
                    scripts.push_back(i->path().string());
            }
            ++i;
        }
    }
    catch (std::exception &/*e*/)
    {
    }
    
    // Create a scriptengine for each of the files, and try to run
    for (uint i = 0; i < scripts.size(); ++i)
    {
        JavascriptEngine* javaScriptInstance = new JavascriptEngine(QString::fromStdString(scripts[i]));
        
        //Register all services to script engine
        PrepareScriptEngine(javaScriptInstance);
        
        startupScripts_.push_back(javaScriptInstance);
        javaScriptInstance->Run();
    }
}

void JavascriptModule::UnloadStartupScripts()
{
    // Stop the startup scripts, if any running
    for (uint i = 0; i < startupScripts_.size(); ++i)
        startupScripts_[i]->Stop();
    for (uint i = 0; i < startupScripts_.size(); ++i)
        delete startupScripts_[i];
    startupScripts_.clear();
}

void JavascriptModule::PrepareScriptEngine(JavascriptEngine* engine)
{
    ServiceMap::iterator iter = services_.begin();
    for(; iter != services_.end(); iter++)
        engine->RegisterService(iter.value(), iter.key());
}

QScriptValue Print(QScriptContext *context, QScriptEngine *engine)
{
    std::cout << "{QtScript} " << context->argument(0).toString().toStdString() << "\n";
    return QScriptValue();
}

QScriptValue ScriptRunFile(QScriptContext *context, QScriptEngine *engine)
{
    JavascriptModule::GetInstance()->RunScript(context->argument(0).toString());
    return QScriptValue();
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(JavascriptModule)
POCO_END_MANIFEST
