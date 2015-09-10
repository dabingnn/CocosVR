#include "HelloWorldScene.h"
#include "AppMacros.h"
#include "extensions/cocos-ext.h"
#include "editor-support/cocostudio/CocoStudio.h"

USING_NS_CC;


Scene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::create();
    
    // add layer as a child to scene
    scene->addChild(layer);
    
    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    {
        _movementLabel = nullptr;
        _originalHeadRotation = Mat4::IDENTITY;
        _originalTranslation = Vec3::ZERO;
    }
    
    initScene();
    scheduleUpdate();
    
    return true;
}

bool HelloWorld::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *evt)
{
    //_isMoving = true;
    return true;
}

void HelloWorld::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *evt)
{
    auto delta = touch->getDelta();
    _moveSpeed += delta.x/10;
    _moveSpeed = clampf(_moveSpeed, 5, 100);
    
}

Vec3 HelloWorld::getPickPosition() const
{
    Vec3 result(-10000,-10000,-10000);
    auto transform = _camera->getNodeToWorldTransform();
    //auto terrainTM = _terrain->getNodeToWorldTransform();
    Vec3 pos(0,0,0);
    transform.transformPoint(&pos);
    Vec3 dir(0,0,-1);
    transform.transformVector(&dir);
    dir.normalize();
    Ray ray(pos, dir);
    float step = _boxAABB._max.x - _boxAABB._min.x;
    const float MAXLENGTH = 12;
    const float MINLENGTH = 4;
    const float WALKSTEP = 0.2;
    float stepX = (_boxAABB._max.x - _boxAABB._min.x);
    float stepY = (_boxAABB._max.y - _boxAABB._min.y);
    float stepZ = (_boxAABB._max.z - _boxAABB._min.z);
    bool isIntersect(false);
    int interSectX; int interSectY; int interSectZ;
    Vec3 intesectPoint;
    for(float index = MINLENGTH; index <= MAXLENGTH;)
    {
        Vec3 stepPos = pos + dir * index * step;
        int x = (stepPos.x)/stepX;
        int y = (stepPos.y)/stepY;
        int z = (stepPos.z)/stepZ;
        x = clampf(x, -50, 50); y = clampf(y, -50, 50); z = clampf(z, -50, 50);
        if(false == _boxArray[x + 50][y + 50][z + 50])
        {
            interSectX = x + 50; interSectY = y + 50; interSectZ = z + 50;
            intesectPoint = Vec3(x * stepX, y * stepY, z * stepZ);
            isIntersect = true;
        }
        else
        {
            break;
        }
        index += WALKSTEP;
    }
    
    if(isIntersect)
    {
        result.x = interSectX;
        result.y = interSectY;
        result.z = interSectZ;
    }
    
    return result;
}

void HelloWorld::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *evt)
{
    auto pickPos = getPickPosition();
    if(pickPos.x >=0 && pickPos.x < 100 && pickPos.y >=0 && pickPos.y < 100 && pickPos.z >=0 && pickPos.z < 100)
    {
        _boxArray[(unsigned int)pickPos.x][(unsigned int)pickPos.y][(unsigned int)pickPos.z] = true;
        float stepX = (_boxAABB._max.x - _boxAABB._min.x);
        float stepY = (_boxAABB._max.y - _boxAABB._min.y);
        float stepZ = (_boxAABB._max.z - _boxAABB._min.z);
        auto sprite = createBox();
        if(sprite)
        {
            sprite->setPosition3D(Vec3((pickPos.x - 50)* stepX, (pickPos.y - 50)* stepY, (pickPos.z - 50)* stepZ));
            this->addChild(sprite);
        }
        
    }
    evt->stopPropagation();
    
}

void HelloWorld::initScene()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    auto vp = Camera::getDefaultViewport();
    _headNode = Node::create();
    
    //use custom camera
    _camera = Camera::createPerspective(60,visibleSize.width/visibleSize.height,0.1f,800);
    _camera->setCameraFlag(CameraFlag::USER1);
    _camera->setPosition3D(Vec3(-1,5.0f,4));
    //_camera->setRotation3D(Vec3(-50,0,0));
    _camera->setFrameBufferObject(Director::getInstance()->getDefaultFBO());
    _camera->setViewport(experimental::Viewport(vp._left,vp._bottom, vp._width, vp._height));
    _headNode->addChild(_camera);
    addChild(_headNode);
    {
        //_boxTextureNames.push_back("vr/Icon.png");
        _boxTextureNames.push_back("vr/GreenSkin.jpg");
        _boxTextureNames.push_back("vr/sand.jpg");
        _boxTextureNames.push_back("vr/road.jpg");
        _selectedTextureNameIndex = 0;
    }
    
    {
//        auto label = Label::createWithTTF(ttfConfig, "Back");
//        
//        auto menuItem = MenuItemLabel::create(label, std::bind(&TestBase::backsUpOneLevel, this));
//        auto menu = Menu::create(menuItem, nullptr);
//        
//        menu->setPosition(Vec2::ZERO);
//        menuItem->setPosition(Vec2(VisibleRect::right().x - 50, VisibleRect::bottom().y + 25));
//        
//        scene->addChild(menu, 1);
        auto label = Label::createWithSystemFont("Switch Box", "Arial", 16);
        auto menuItem = MenuItemLabel::create(label,
                                              [this](Ref* ref)
                                              {
                                                  _selectedTextureNameIndex++;
                                                  _selectedTextureNameIndex = _selectedTextureNameIndex%_boxTextureNames.size();
                                                  auto sprite = createBox();
                                                  this->addChild(sprite);
                                                  this->removeChild(_objectNode);
                                                  _objectNode = sprite;
                                                  _objectNode->setPosition3D(Vec3::ZERO);
                                                  _objectNode->setOpacity(127);
                                              }
                                              );
        auto menu = Menu::create(menuItem, nullptr);
        menu->setPosition(Vec2::ZERO);
        menuItem->setPosition(Vec2(visibleSize.width - 50, 25));
        
        this->addChild(menu);
    }
    
    //_camera->runAction(RepeatForever::create(RotateBy::create(10, Vec3(0,50,0))));
    
    {
        _originalHeadRotation = _headNode->getNodeToParentTransform();
        _originalHeadRotation.getTranslation(&_originalTranslation);
        Quaternion q;
        _originalHeadRotation.getRotation(&q);
        Mat4::createRotation(q, &_originalHeadRotation);
        Vec3 forward(0,0,-1);
        _originalHeadRotation.transformVector(&forward);
        forward.y = 0;
        forward.normalize();
        Mat4::createLookAt(Vec3::ZERO, forward, Vec3(0,1,0), &_originalHeadRotation);
        
    }
    
    {
        auto sprite = Sprite3D::create("vr/box.c3t", "vr/Icon.png");
        _boxAABB = sprite->getAABB();
        memset(&_boxArray[0][0][0], 0, sizeof(_boxArray));
        for(int indexX = 0; indexX < 100; ++indexX)
        {
            for(int indexZ = 0; indexZ < 100; ++indexZ)
            {
                _boxArray[indexX][49][indexZ] = true;
            }
        }
    }
    
    Terrain::DetailMap r("TerrainTest/Grass2.jpg"),g("TerrainTest/Grass2.jpg"),b("TerrainTest/Grass2.jpg"),a("TerrainTest/Grass2.jpg");
    
    Terrain::TerrainData data("TerrainTest/heightmap16.jpg","TerrainTest/alphamap.png",r,g,b,a);
    
    _terrain = Terrain::create(data,Terrain::CrackFixedType::SKIRT);
    _terrain->setLODDistance(3.2f,6.4f,9.6f);
    _terrain->setMaxDetailMapAmount(4);
    addChild(_terrain);
    _terrain->setCameraMask(2);
    _terrain->setDrawWire(false);
    
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    //add label
    {
        auto label = Label::createWithSystemFont("", "Arial", 8);
        label->setAnchorPoint(Vec2(0, 1.0));
        label->setPosition(0, visibleSize.height);
        _movementLabel = label;
        addChild(label);
        
        auto sprite = Sprite3D::create("vr/box.c3t", "vr/GreenSkin.jpg");
        sprite->setCameraMask((unsigned short)CameraFlag::USER1);
        sprite->setPosition3D(Vec3::ZERO);
        sprite->setColor(Color3B(0,255,0));
        sprite->setScale(0.01);
        _camera->addChild(sprite);
        sprite->setPosition3D(Vec3(0,0, -1));
    }
    //add demo box
    {
        _objectNode = createBox();
        _objectNode->setPosition3D(Vec3::ZERO);
        //sprite->setColor(Color3B(0,255,0));
        _objectNode->setOpacity(127);
        this->addChild(_objectNode);
    }
    
}

//void HelloWorld::initScene()
//{
//    Size visibleSize = Director::getInstance()->getVisibleSize();
//    auto vp = Camera::getDefaultViewport();
//    auto sceneNode = CSLoader::createNode("res/Scene3DNoParticle.csb");
//    sceneNode->setCameraMask((unsigned short)CameraFlag::USER1, true);
//    addChild(sceneNode);
//    _headNode = sceneNode->getChildByTag(57);
//
//    {
//        _originalHeadRotation = _headNode->getNodeToParentTransform();
//        _originalHeadRotation.getTranslation(&_originalTranslation);
//        Quaternion q;
//        _originalHeadRotation.getRotation(&q);
//        Mat4::createRotation(q, &_originalHeadRotation);
//        Vec3 forward(0,0,-1);
//        _originalHeadRotation.transformVector(&forward);
//        forward.y = 0;
//        forward.normalize();
//        Mat4::createLookAt(Vec3::ZERO, forward, Vec3(0,1,0), &_originalHeadRotation);
//
//    }
//
//    {
//        _camera = Camera::createPerspective(60,visibleSize.width/visibleSize.height,0.1f,800);
//        _camera->setCameraFlag(CameraFlag::USER1);
//        //
//        //        _camera->setPosition3D(Vec3(-0.01,0,0));
//        _camera->setFrameBufferObject(Director::getInstance()->getDefaultFBO());
//        _camera->setViewport(experimental::Viewport(vp._left,vp._bottom, vp._width, vp._height));
//        _headNode->addChild(_camera);
//
//    }
//
//    //add skybox
//    {
//        auto textureCube = TextureCube::create("skybox/left.jpg", "skybox/right.jpg",
//                                           "skybox/top.jpg", "skybox/bottom.jpg",
//                                           "skybox/front.jpg", "skybox/back.jpg");
//        auto skyBox = Skybox::create();
//        skyBox->retain();
//
//        skyBox->setTexture(textureCube);
//        addChild(skyBox);
//        skyBox->setCameraMask((unsigned short)CameraFlag::USER1);
//    }
//
//    //add label
//    {
//        auto label = Label::createWithSystemFont("", "Arial", 8);
//        label->setAnchorPoint(Vec2(0, 1.0));
//        label->setPosition(0, visibleSize.height);
//        //label->setCameraMask((unsigned short)CameraFlag::USER2);
//        _movementLabel = label;
//        addChild(label);
//
////        auto camera = Camera::createPerspective(60,visibleSize.width/visibleSize.height * 0.5,0.1f,800);
////        camera->setCameraFlag(CameraFlag::USER2);
////        camera->setFrameBufferObject(Director::getInstance()->getDefaultFBO());
////        camera->setViewport(experimental::Viewport(vp._left,vp._bottom, vp._width/2, vp._height));
////        _headNode->addChild(camera);
//
////        auto node = Node::create();
////        node->setPosition3D(_headNode->getPosition3D());
////        node->setRotationQuat(_headNode->getRotationQuat());
////        sceneNode->addChild(node);
////        node->addChild(camera);
//    }
//    auto listener = EventListenerTouchOneByOne::create();
//    listener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
//    listener->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);//    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
//    CCASSERT(_headNode, "");
//}

void HelloWorld::update(float delta)
{
    Mat4 transform = Director::getInstance()->getHeadTransform();
    Quaternion q;
    transform.getRotation(&q);
    q.inverse();
    Vec3 v = Director::getInstance()->getDeviceMove();
    auto v2 = v;
    if(_headNode)
    {
        //CCLOG("head rotation is %lf, %lf, %lf, %lf", q.x, q.y,q.z,q.w);
        _headNode->setRotationQuat(q);
        _originalHeadRotation.transformVector(v.x, v.y, v.z, 0, &v2);
        _headNode->setPosition3D(_originalTranslation + v2 * _moveSpeed);
        v2 = _headNode->getPosition3D();
    }
    //CCLOG("Device move is %.6f, %.6f, %.6f", v.x, v.y, v.z);
    if(_movementLabel)
    {
        auto str = StringUtils::format("Position %.4f, %.4f, %.4f, Speed %.4f", v.x, v.y, v.z, _moveSpeed);
        _movementLabel->setString(str);
    }
    
    auto pickPos = getPickPosition();
    if(pickPos.x >=0 && pickPos.x < 100 && pickPos.y >=0 && pickPos.y < 100 && pickPos.z >=0 && pickPos.z < 100)
    {
        float stepX = (_boxAABB._max.x - _boxAABB._min.x);
        float stepY = (_boxAABB._max.y - _boxAABB._min.y);
        float stepZ = (_boxAABB._max.z - _boxAABB._min.z);
        _objectNode->setPosition3D(Vec3((pickPos.x - 50)* stepX, (pickPos.y - 50)* stepY, (pickPos.z - 50)* stepZ));
    }
    
//    {
//        Vec3 result(-10000,-10000,-10000);
//        auto transform = _camera->getNodeToWorldTransform();
//        //auto terrainTM = _terrain->getNodeToWorldTransform();
//        Vec3 pos(0,0,0);
//        transform.transformPoint(&pos);
//        Vec3 dir(0,0,-1);
//        transform.transformVector(&dir);
//        dir.normalize();
//        Ray ray(pos, dir);
//        _objectNode->setPosition3D(Vec3(pos + dir * 5));
//    }
    
    {
        auto transform = _camera->getNodeToWorldTransform();
        Vec3 pos(0,0,0);
        transform.transformPoint(&pos);
        Vec3 dir(0,0,-1);
        transform.transformVector(&dir);
        dir.normalize();
        Ray ray(pos, dir);
        Vec3 pt;
        if(_terrain->getIntersectionPoint(ray, pt))
        {
            CCLOG("Terrain is picked %.3f, %.3f, %.3f", pt.x, pt.y, pt.z);
        }
    }
    
}

Node* HelloWorld::createBox()
{
    auto sprite = Sprite3D::create("vr/box.c3t", _boxTextureNames[_selectedTextureNameIndex]);
    sprite->setCameraMask((unsigned short)CameraFlag::USER1);
    //sprite->setPosition3D(pos);
    return sprite;
}

void HelloWorld::menuCloseCallback(Ref* sender)
{
    Director::getInstance()->end();
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
