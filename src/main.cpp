#include "ofMain.h"
#include "virtual_chat_client_app.hpp"

int main() {
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new VirtualChatClientApp());
}
