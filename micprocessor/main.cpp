#include "includes.ihh"

// include the modules
#include "alsoundprocessing.hpp"

#ifdef SOUNDPROCESSING_IS_REMOTE
# define ALCALL
#else
# ifdef _WIN32
#  define ALCALL __declspec(dllexport)
# else
#  define ALCALL
# endif
#endif

#define BOOST_SIGNALS_NO_DEPRECATION_WARNING

extern "C"
{
  ALCALL int _createModule(boost::shared_ptr<AL::ALBroker> pBroker)
  {
    // init broker with the main broker instance
    // from the parent executable
    AL::ALBrokerManager::setInstance(pBroker->fBrokerManager.lock());
    AL::ALBrokerManager::getInstance()->addBroker(pBroker);

    // create module instances
    AL::ALModule::createModule<microphone>(pBroker, 
                                           "microphone");
    return 0;
  }

  ALCALL int _closeModule()
  {
    return 0;
  }
} // extern "C"

int main(int argc, char *argv[] )
{
    curl_global_init(CURL_GLOBAL_ALL);
    // pointer to createModule
    TMainType sig;
    sig = &_createModule;
    // call main
    ALTools::mainFunction("alsoundprocessing", argc, argv, sig);
}
