#include "virtual_chat_client_app.hpp"

void VirtualChatClientApp::setup() {
    ofBackground(0, 0, 0);
    ofEnableSmoothing();
    ofDisableArbTex();

    //========================3Dモデルの初期化========================//

    room.loadModel("showroom_3ds.3ds");
    room.setScale(20, 20, 20);
    room.setRotation(0, 180, 1, 0, 0);
    room.setPosition(0, 0, 0);

    //======================アイコン画像の初期化======================//

    icons["joy"].load("joy.png");
    icons["sorrow"].load("sorrow.png");
    icons["anger"].load("anger.png");
    icons["surprise"].load("surprise.png");

    for (auto &icon : icons) {
        icon.second.resize(20, 20);
    }

    //======================カメラ,ライトの初期化=====================//

    light.setPointLight();
    light.setPosition(room.getSceneCenter());
    light.setDiffuseColor(ofFloatColor(1, 1, 1));
    light.setAmbientColor(ofFloatColor(0, 0, 0));
    light.setSpecularColor(ofFloatColor(1, 1, 1));

    //===========================GUIの初期化==========================//

    gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::TOP_LEFT);
    gui->addButton("connect");
    gui->addButton("disconnect");
    gui->addTextInput("local port", "11888");
    gui->addTextInput("remote address", "172.20.10.3");
    gui->addTextInput("remote port", "11999");
    gui->addTextInput("message", "");

    gui->onButtonEvent([this](const ofxDatGuiButtonEvent &event) {
        if (event.target->is("connect")) {
            connect();
        }
        if (event.target->is("disconnect")) {
            disconnect();
        }
    });

    gui->onTextInputEvent([this](const ofxDatGuiTextInputEvent &event) {
        if (event.target->is("message") && udpSender) {
            udpSender->async_send([](...) {}, id, "message", event.text);
        }
    });

    font.load("Verdana", 24);

    //====================表情認識スレッドを立ち上げ==================//

    //==========================TCP通信の設定========================//

    random_device randomDevice;
    mt19937 generator(randomDevice());
    uniform_int_distribution<char> distribution('a', 'z');

    generate_n(back_inserter(id), 5, [&]() { return distribution(generator); });
}

void VirtualChatClientApp::update() {
    glService.run();
    glService.reset();

    //=========================3Dモデルの更新=========================//

    if (avatars.find(id) != avatars.end() && udpSender) {
        if (ofGetKeyPressed(OF_KEY_UP)) {
            auto direction = ofPoint(0, 0, -1).getRotated(
                get<0>(avatars[id]).getRotationAngle(1),
                get<0>(avatars[id]).getRotationAxis(1));
            auto position = get<0>(avatars[id]).getPosition() + direction * +20;

            udpSender->async_send([](...) {}, id, "position", position);
        } else if (ofGetKeyPressed(OF_KEY_DOWN)) {
            auto direction = ofPoint(0, 0, -1).getRotated(
                get<0>(avatars[id]).getRotationAngle(1),
                get<0>(avatars[id]).getRotationAxis(1));
            auto position = get<0>(avatars[id]).getPosition() + direction * -20;

            udpSender->async_send([](...) {}, id, "position", position);
        }

        if (ofGetKeyPressed(OF_KEY_RIGHT)) {
            auto rotation = get<0>(avatars[id]).getRotationAngle(1) - 2;

            udpSender->async_send([](...) {}, id, "rotation", rotation);
        } else if (ofGetKeyPressed(OF_KEY_LEFT)) {
            auto rotation = get<0>(avatars[id]).getRotationAngle(1) + 2;

            udpSender->async_send([](...) {}, id, "rotation", rotation);
        }
    }

    room.update();

    for (auto &avatar : avatars) {
        get<0>(avatar.second).update();
    }

    //==========================カメラの更新==========================//

    if (avatars.find(id) != avatars.end()) {
        localCamera.setPosition(get<0>(avatars[id]).getPosition() +
                                ofPoint(0, 0, -1).getRotated(
                                    get<0>(avatars[id]).getRotationAngle(1),
                                    get<0>(avatars[id]).getRotationAxis(1)) *
                                    100 +
                                ofPoint(0, 600, 0));

        localCamera.setOrientation(
            ofPoint(0, get<0>(avatars[id]).getRotationAngle(1), 0));

        globalCamera.setPosition(get<0>(avatars[id]).getPosition() +
                                 ofPoint(0, 0, -1).getRotated(
                                     get<0>(avatars[id]).getRotationAngle(1),
                                     get<0>(avatars[id]).getRotationAxis(1)) *
                                     -1000 +
                                 ofPoint(0, 600, 0));

        globalCamera.setOrientation(
            ofPoint(0, get<0>(avatars[id]).getRotationAngle(1), 0));
    }
}

void VirtualChatClientApp::draw() {
    ofEnableDepthTest();
    ofEnableLighting();

    cameraType == CameraType::Global ? globalCamera.begin()
                                     : localCamera.begin();

    light.enable();

    ofSetColor(255, 255, 255, 255);

    room.drawFaces();

    for (auto &avatar : avatars) {
        get<0>(avatar.second).drawFaces();

        if (!get<1>(avatar.second).empty()) {
            ofPushMatrix();

            ofTranslate(get<0>(avatar.second).getPosition() +
                        ofPoint(0, 600, 0));

            ofRotateYDeg(get<0>(avatar.second).getRotationAngle(1) + 180);

            auto bb = font.getStringBoundingBox(get<1>(avatar.second), 0, 0);

            ofSetColor(255, 255, 255, 127);
            ofFill();

            ofDrawBox(0, 0, 0, bb.getWidth() + 20, bb.getHeight() + 20, 10);

            ofSetColor(0, 0, 0, 255);
            ofNoFill();

            ofDrawBox(0, 0, 0, bb.getWidth() + 20, bb.getHeight() + 20, 10);

            ofTranslate(0, 0, 10);

            ofSetColor(0, 0, 0, 255);

            font.drawString(get<1>(avatar.second), -bb.getWidth() / 2,
                            -bb.getHeight() / 2);

            ofPopMatrix();
        }

        if (icons.find(get<0>(get<2>(avatar.second))) != icons.end()) {
            ofPushMatrix();

            ofTranslate(get<0>(avatar.second).getPosition() +
                        ofPoint(0, 650, 0));

            ofRotateYDeg(ofGetElapsedTimef() * 180);

            ofSetColor(255, 255, 255, 255 * get<1>(get<2>(avatar.second)));

            icons[get<0>(get<2>(avatar.second))].draw(0, 0);

            ofPopMatrix();
        }
    }

    light.disable();

    cameraType == CameraType::Global ? globalCamera.end() : localCamera.end();

    ofDisableLighting();
    ofDisableDepthTest();

    if (avatars.find(id) != avatars.end()) {
        ofSetColor(255, 255, 255, 255);

        stringstream info;
        info << "estimated emotion: " << get<0>(get<2>(avatars[id]))
             << ", likelihood: " << get<1>(get<2>(avatars[id])) << endl;

        ofDrawBitmapString(info.str(), 0, ofGetHeight());
    }
}

void VirtualChatClientApp::exit() { disconnect(); }

void VirtualChatClientApp::keyPressed(int key) {
    if (key == ' ') {
        cameraType = cameraType == CameraType::Global ? CameraType::Local
                                                      : CameraType::Global;
    }
}

void VirtualChatClientApp::connect() {
    boost::asio::ip::udp::endpoint localEndpoint(
        boost::asio::ip::address::from_string("127.0.0.1"),
        stoi(gui->getTextInput("local port")->getText()));
    boost::asio::ip::udp::endpoint remoteEndpoint(
        boost::asio::ip::address::from_string(
            gui->getTextInput("remote address")->getText()),
        stoi(gui->getTextInput("remote port")->getText()));

    udpSender =
        make_unique<UDPSender>(sendService, localEndpoint, remoteEndpoint);

    udpSender->async_send([](...) {}, id, "created");

    sender = make_unique<thread>([this]() {
        boost::asio::io_service::work work(sendService);
        sendService.run();
    });

    udpReceiver = make_unique<UDPReceiver>(receiveService, localEndpoint);

    udpReceiver->async_receive(
        boost::bind(&VirtualChatClientApp::handleReceive, this,
                    boost::asio::placeholders::bytes_transferred));

    receiver = make_unique<thread>([this]() { receiveService.run(); });

    if (videoGrabber.setup(1280, 720)) {
        py::module::import("sys").attr("path").cast<py::list>().append(
            "../../../../src");

        detectFaceService.post(
            boost::bind(&VirtualChatClientApp::detectFace, this));

        faceDetector =
            make_unique<thread>([this]() { detectFaceService.run(); });
    }
}

void VirtualChatClientApp::disconnect() {
    if (udpSender) {
        udpSender->send(id, "destroyed");
    }

    sendService.stop();
    receiveService.stop();
    detectFaceService.stop();

    if (sender && sender->joinable()) {
        sender->join();
    }
    if (receiver && receiver->joinable()) {
        receiver->join();
    }
    if (faceDetector && faceDetector->joinable()) {
        faceDetector->join();
    }

    udpSender.reset();
    udpReceiver.reset();
}

void VirtualChatClientApp::handleReceive(size_t size) {
    string str(udpReceiver->buffer.data(), size);
    cout << str << endl;

    glService.post([=]() {
        stringstream sstream(str);

        string id, type;
        if (!(sstream >> id >> type)) return;

        if (avatars.find(id) == avatars.end()) {
            get<0>(avatars[id]).loadModel("astroBoy_walk.dae");
            get<0>(avatars[id]).setScale(1, 1, 1);
            get<0>(avatars[id]).setRotation(0, 180, 1, 0, 0);
            get<0>(avatars[id]).setPosition(0, 0, 0);
            get<0>(avatars[id]).setLoopStateForAllAnimations(OF_LOOP_NORMAL);
            get<0>(avatars[id]).playAllAnimations();
            get<1>(avatars[id]) = "";
            get<2>(avatars[id]) = forward_as_tuple("unknown", 0.0);
        } else {
            if (type == "destroyed") {
                avatars.erase(id);
            } else if (type == "position") {
                ofVec3f position;
                sstream >> position;

                get<0>(avatars[id])
                    .setPosition(position.x, position.y, position.z);
            } else if (type == "rotation") {
                float rotation;
                sstream >> rotation;

                get<0>(avatars[id]).setRotation(1, rotation, 0, 1, 0);
            } else if (type == "message") {
                string message;
                getline(sstream, message);

                get<1>(avatars[id]) = message;
            } else if (type == "emotion") {
                tuple<string, float> emotion;
                sstream >> emotion;

                get<2>(avatars[id]) = emotion;
            }
        }
    });

    udpReceiver->async_receive(
        boost::bind(&VirtualChatClientApp::handleReceive, this,
                    boost::asio::placeholders::bytes_transferred));
}

void VirtualChatClientApp::detectFace() {
    glService.post([this]() { videoGrabber.update(); });

    // 現在キャプチャしている画像を保存
    ofSaveImage(videoGrabber.getPixels(), "face.jpg");

    auto faceDetection = py::module::import("face_detection");

    // detectFace関数を実行 (Google Cloud Vision APIに感情推定をリクエスト)
    auto emotion =
        faceDetection.attr("detectFace")("../../../../bin/data/face.jpg")
            .cast<tuple<string, float>>();

    udpSender->async_send([](...) {}, id, "emotion", emotion);

    detectFaceService.post(
        boost::bind(&VirtualChatClientApp::detectFace, this));
}
