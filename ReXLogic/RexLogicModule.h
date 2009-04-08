// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_h
#define incl_RexLogicModule_h

#include "ModuleInterface.h"
#include "NetworkEventHandler.h"
#include "RexServerConnection.h"

namespace RexLogic
{
    typedef boost::shared_ptr<RexServerConnection> RexServerConnectionPtr;

    //! interface for modules
    class MODULE_API RexLogicModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        RexLogicModule();
        virtual ~RexLogicModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();

        virtual void Update(Core::f64 frametime);
        
        virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data);
        
        MODULE_LOGGING_FUNCTIONS;

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_WorldLogic;

        RexServerConnectionPtr GetServerConnection() const { return rexserver_connection_; }

    private:        
        NetworkEventHandler *network_handler_; 
        
        RexServerConnectionPtr rexserver_connection_;

        typedef boost::function<bool(Core::event_id_t,Foundation::EventDataInterface*)> LogicEventHandlerFunction;
        typedef std::map<Core::event_category_id_t, LogicEventHandlerFunction> LogicEventHandlerMap;
        LogicEventHandlerMap event_handlers_;
        
        Console::CommandResult TestMoveForward(const Core::StringVector &params);
    };
}

#endif
