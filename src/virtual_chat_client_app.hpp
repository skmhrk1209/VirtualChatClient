/*================================================================//

    2018/06/10 Software Development

    Virtual Chat Client Application

    [github](https://github.com/skmhrk1209/VirtualChatClient)

    機能1: ユーザの感情推定(喜び、悲しみ、怒り、驚き)を行い、アイコンで表示

//================================================================*/

#pragma once

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <random>
#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofxDatGui.h"
#include "udp_receiver.hpp"
#include "udp_sender.hpp"
#include "utility.hpp"

namespace py = pybind11;

//======================アプリケーション本体======================//

class VirtualChatClientApp : public ofBaseApp {
   public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;
    void keyPressed(int) override;

    void connect();
    void disconnect();

    void handleReceive(size_t);
    void detectFace();

    // アバターの3Dモデル
    using Avatar = tuple<ofxAssimpModelLoader, string, tuple<string, float>>;
    unordered_map<string, Avatar> avatars;

    // 部屋の3Dモデル
    // ユーザの感情に対応するアイコン画像
    ofxAssimpModelLoader room;
    unordered_map<string, ofImage> icons;

    // カメラやライティング
    enum class CameraType { Global, Local } cameraType;
    ofCamera localCamera;
    ofCamera globalCamera;
    ofLight light;

    // GUI
    unique_ptr<ofxDatGui> gui;
    ofTrueTypeFont font;

    // 表情認識のためのスレッド
    ofVideoGrabber videoGrabber;

    // pythonインタプリタ
    py::scoped_interpreter interpreter;

    // TCP通信のためのクラス
    boost::asio::io_service glService;
    boost::asio::io_service sendService;
    boost::asio::io_service receiveService;
    boost::asio::io_service detectFaceService;

    unique_ptr<UDPSender> udpSender;
    unique_ptr<UDPReceiver> udpReceiver;

    unique_ptr<thread> sender;
    unique_ptr<thread> receiver;
    unique_ptr<thread> faceDetector;

    string id;
};
