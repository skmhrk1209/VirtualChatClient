#include "virtual_chat_client_app.hpp"

void VirtualChatClientApp::setup() {
    //=================================glの設定======================================//

    ofBackground(0, 0, 0);
    ofEnableSmoothing();
    ofDisableArbTex();

    //=================================GUIの設定======================================//

    gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::TOP_LEFT);
    gui->addButton("connect");
    gui->addButton("disconnect");
    gui->addTextInput("local port", "11888");
    gui->addTextInput("remote address", "172.20.10.3");
    gui->addTextInput("remote port", "11999");
    gui->addTextInput("message", "");
    gui->addDropdown("avatars", {"boy"})->select(0);

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
            udpSender->async_send([](...) {}, id,
                                  gui->getDropdown("avatars")->getLabel(),
                                  "message", event.text);
        }
    });

    font.load("Verdana", 24);

    //=================================3Dモデル初期化======================================//

    scene.loadModel("showroom_3ds.3ds");
    scene.setScale(20, 20, 20);
    scene.setRotation(0, 180, 1, 0, 0);
    scene.setPosition(0, 0, 0);

    models["boy"].loadModel("astroBoy_walk.dae");
    models["boy"].setScale(1, 1, 1);
    models["boy"].setRotation(0, 180, 1, 0, 0);
    models["boy"].setPosition(0, 0, 0);
    models["boy"].setLoopStateForAllAnimations(OF_LOOP_NORMAL);
    models["boy"].playAllAnimations();

    lights.emplace_back();
    lights.back().setPointLight();
    lights.back().setPosition(scene.getSceneCenter());
    lights.back().setDiffuseColor(ofFloatColor(1, 1, 1));
    lights.back().setAmbientColor(ofFloatColor(0, 0, 0));
    lights.back().setSpecularColor(ofFloatColor(1, 1, 1));

    //=================================アイコン画像初期化======================================//

    icons["joy"].load("joy.png");
    icons["sorrow"].load("sorrow.png");
    icons["anger"].load("anger.png");
    icons["surprise"].load("surprise.png");

    for (auto &icon : icons) {
        icon.second.resize(20, 20);
    }

    //=================================ユーザID生成======================================//

    random_device randomDevice;
    mt19937 generator(randomDevice());
    uniform_int_distribution<char> distribution('a', 'z');

    generate_n(back_inserter(id), 5, [&]() { return distribution(generator); });
}

void VirtualChatClientApp::update() {
    //=================================glタスクの実行======================================//

    glService.run();
    glService.reset();

    //=================================モデルの更新======================================//

    if (avatars.find(id) != avatars.end() && udpSender) {
        if (ofGetKeyPressed(OF_KEY_UP)) {
            auto direction = ofPoint(0, 0, -1).getRotated(
                get<0>(avatars[id]).getRotationAngle(1),
                get<0>(avatars[id]).getRotationAxis(1));
            auto position = get<0>(avatars[id]).getPosition() + direction * +20;

            udpSender->async_send([](...) {}, id,
                                  gui->getDropdown("avatars")->getLabel(),
                                  "position", position);
        } else if (ofGetKeyPressed(OF_KEY_DOWN)) {
            auto direction = ofPoint(0, 0, -1).getRotated(
                get<0>(avatars[id]).getRotationAngle(1),
                get<0>(avatars[id]).getRotationAxis(1));
            auto position = get<0>(avatars[id]).getPosition() + direction * -20;

            udpSender->async_send([](...) {}, id,
                                  gui->getDropdown("avatars")->getLabel(),
                                  "position", position);
        }

        if (ofGetKeyPressed(OF_KEY_RIGHT)) {
            auto rotation = get<0>(avatars[id]).getRotationAngle(1) - 2;

            udpSender->async_send([](...) {}, id,
                                  gui->getDropdown("avatars")->getLabel(),
                                  "rotation", rotation);
        } else if (ofGetKeyPressed(OF_KEY_LEFT)) {
            auto rotation = get<0>(avatars[id]).getRotationAngle(1) + 2;

            udpSender->async_send([](...) {}, id,
                                  gui->getDropdown("avatars")->getLabel(),
                                  "rotation", rotation);
        }
    }

    scene.update();

    for (auto &avatar : avatars) {
        get<0>(avatar.second).update();
    }

    //=================================カメラの更新======================================//

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

    for (auto &light : lights) {
        light.enable();
    }

    ofSetColor(255, 255, 255, 255);

    scene.drawFaces();

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

    for (auto &light : lights) {
        light.disable();
    }

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
    //=================================視点切り替え======================================//
    if (key == ' ') {
        cameraType = cameraType == CameraType::Global ? CameraType::Local
                                                      : CameraType::Global;
    }
}

//=================================サーバとのconnectionを張る======================================//
void VirtualChatClientApp::connect() {
    // 一度connectionを貼ったらアバターの変更はできない
    gui->getDropdown("avatars")->setEnabled(false);

    //=================================サーバのアドレスとポートを指定======================================//

    boost::asio::ip::udp::endpoint localEndpoint(
        boost::asio::ip::address::from_string("127.0.0.1"),
        stoi(gui->getTextInput("local port")->getText()));
    boost::asio::ip::udp::endpoint remoteEndpoint(
        boost::asio::ip::address::from_string(
            gui->getTextInput("remote address")->getText()),
        stoi(gui->getTextInput("remote port")->getText()));

    //=================================senderの設定======================================//

    // senderを生成
    udpSender =
        make_unique<UDPSender>(sendService, localEndpoint, remoteEndpoint);

    // createメッセージを非同期送信
    udpSender->async_send([](...) {}, id,
                          gui->getDropdown("avatars")->getLabel(), "created");

    // send完了ハンドラの実行スレッド生成
    sender = make_unique<thread>([this]() {
        boost::asio::io_service::work work(sendService);
        sendService.run();
    });

    //=================================receiverの設定======================================//

    // receiverの生成
    udpReceiver = make_unique<UDPReceiver>(receiveService, localEndpoint);

    // 非同期受信開始
    udpReceiver->async_receive(
        boost::bind(&VirtualChatClientApp::handleReceive, this,
                    boost::asio::placeholders::bytes_transferred));

    // receive完了ハンドラの実行スレッド生成
    receiver = make_unique<thread>([this]() { receiveService.run(); });

    //=================================faceDetectorの設定======================================//

    if (!videoGrabber.isInitialized() && videoGrabber.setup(1280, 720)) {
        py::module::import("sys").attr("path").cast<py::list>().append(
            "../../../../src");

        // 表情認識タスクをポスト
        detectFaceService.post(
            boost::bind(&VirtualChatClientApp::detectFace, this));

        // 表情認識実行スレッド生成
        faceDetector =
            make_unique<thread>([this]() { detectFaceService.run(); });
    }
}

//=================================サーバとのconnectionを切る======================================//
void VirtualChatClientApp::disconnect() {
    // destroyedメッセージを送信
    if (udpSender) {
        udpSender->send(id, "destroyed");
    }

    // 全サービスの停止
    sendService.stop();
    receiveService.stop();
    detectFaceService.stop();

    // スレッド終了待ち
    if (sender && sender->joinable()) {
        sender->join();
    }
    if (receiver && receiver->joinable()) {
        receiver->join();
    }
    if (faceDetector && faceDetector->joinable()) {
        faceDetector->join();
    }

    // senderとreceiverをキル
    udpSender.reset();
    udpReceiver.reset();

    // 全アバターの削除
    avatars.clear();

    // 全サービスのリセット
    sendService.reset();
    receiveService.reset();
    detectFaceService.reset();

    // アバターの変更を可能に
    gui->getDropdown("avatars")->setEnabled(true);
}

//=================================受信完了ハンドラ======================================//
void VirtualChatClientApp::handleReceive(size_t size) {
    string str(udpReceiver->buffer.data(), size);
    cout << str << endl;

    stringstream sstream(str);

    string id, model, type;
    if (!(sstream >> id >> model >> type)) return;

    // 未知のidは新規作成
    if (avatars.find(id) == avatars.end()) {
        avatars.emplace(
            std::piecewise_construct, std::forward_as_tuple(id),
            std::forward_as_tuple(models[model], "",
                                  forward_as_tuple("unknown", 0.0)));
    }

    if (type == "destroyed") {
        avatars.erase(id);
    } else if (type == "position") {
        ofVec3f position;
        sstream >> position;

        get<0>(avatars[id]).setPosition(position.x, position.y, position.z);
    } else if (type == "rotation") {
        float rotation;
        sstream >> rotation;

        get<0>(avatars[id]).setRotation(1, rotation, 0, 1, 0);
    } else if (type == "message") {
        sstream.ignore();

        string message;
        getline(sstream, message);

        get<1>(avatars[id]) = message;
    } else if (type == "emotion") {
        tuple<string, float> emotion;
        sstream >> emotion;

        get<2>(avatars[id]) = emotion;
    }

    // 続けて非同期受信
    udpReceiver->async_receive(
        boost::bind(&VirtualChatClientApp::handleReceive, this,
                    boost::asio::placeholders::bytes_transferred));
}

//=================================表情認識======================================//
void VirtualChatClientApp::detectFace() {
    // glのメインスレッド以外でgl干渉できないので遠回り
    glService.post([this]() { videoGrabber.update(); });

    // 現在キャプチャしている画像を保存
    ofSaveImage(videoGrabber.getPixels(), "face.jpg");

    auto faceDetection = py::module::import("face_detection");

    // detectFace関数を実行 (Google Cloud Vision APIに感情推定をリクエスト)
    auto emotion =
        faceDetection.attr("detectFace")("../../../../bin/data/face.jpg")
            .cast<tuple<string, float>>();

    // 推定された感情を非同期送信
    udpSender->async_send([](...) {}, id,
                          gui->getDropdown("avatars")->getLabel(), "emotion",
                          emotion);

    // 続けて表情認識
    detectFaceService.post(
        boost::bind(&VirtualChatClientApp::detectFace, this));
}
