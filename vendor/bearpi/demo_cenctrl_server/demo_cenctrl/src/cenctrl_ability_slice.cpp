/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include <stdio.h>
#include <string.h>
#include <common/screen.h>

#include "cenctrl_ability_slice.h"
#include "ability_manager.h"
#include "socket_service.h"
#include "socket_udp.h"
#include "cenctrl_log.h"

namespace OHOS {
REGISTER_AS(CenctrlAbilitySlice)


UILabel* g_labelTemp_ { nullptr};
UILabel* g_labelLux_ { nullptr};
UILabel* g_labelHum_ { nullptr};

List<ItemCtrl*>* CtrlCallBack::pItemListeners_ = { nullptr};

void CtrlCallBack::SetInstancePram(List<ItemCtrl*>* itemList)
{
    if (itemList) {
        pItemListeners_ = itemList;
    }
}

void CtrlCallBack::CtrlOnConnect(char* devName, int status)
{
    ListNode<ItemCtrl*>* item = pItemListeners_->Begin();
    while (item != pItemListeners_->End()) {
        if (strcmp(item->data_->devName_.c_str(), devName) == 0) {
            item->data_->OnConnect(devName, status);
            break;
        }
        item = item->next_;
    }
}

void CtrlCallBack::CtrlOnDisconnect(char* devName, int status)
{
    ListNode<ItemCtrl*>* item = pItemListeners_->Begin();
    while (item != pItemListeners_->End()) {
        if (strcmp(item->data_->devName_.c_str(), devName) == 0) {
            item->data_->OnDisconnect(devName, status);
            break;
        }
        item = item->next_;
    }
}

void CtrlCallBack::CtrlOn(char* devName, int status)
{
    ListNode<ItemCtrl*>* item = pItemListeners_->Begin();
    while (item != pItemListeners_->End()) {
        if (strcmp(item->data_->devName_.c_str(), devName) == 0) {
            item->data_->On(devName, status);
            break;
        }
        item = item->next_;
    }
}

void CtrlCallBack::CtrlOff(char* devName, int status)
{
    ListNode<ItemCtrl*>* item = pItemListeners_->Begin();
    while (item != pItemListeners_->End()) {
        if (strcmp(item->data_->devName_.c_str(), devName) == 0) {
            item->data_->Off(devName, status);
            break;
        }
        item = item->next_;
    }
}

CenctrlAbilitySlice::~CenctrlAbilitySlice()
{
    SocketServiceDelete();
    SocketUdpDelBC();

    DeleteChildren(swipeView_);

    ListNode<EventListener*>* listen = listeners_.Begin();
    while(listen != listeners_.End()) {
        delete listen->data_;
        listen = listen->next_;
    }
    listeners_.Clear();

    ListNode<ClickListener*>* click = clickListeners_.Begin();
    while(click != clickListeners_.End()) {
        delete click->data_;
        click = click->next_;
    }
    clickListeners_.Clear();

    ListNode<ItemCtrl*>* item = itemListeners_.Begin();
    while(item != itemListeners_.End()) {
        delete item->data_;
        item = item->next_;
    }
    itemListeners_.Clear();

    if(swipeListener_) {
        delete swipeListener_;
        swipeListener_ = nullptr;
    }
}

void CenctrlAbilitySlice::SetBackground()
{
    uiImageView_ = new UIImageView();
    uiImageView_->SetPosition(0, 0, ROOTVIEW_W, ROOTVIEW_H);
    uiImageView_->SetSrc(RES_BACKGROUND);
    rootview_->Add(uiImageView_);
}

void CenctrlAbilitySlice::SetUpView()
{
    SetUpSwipeView();
    SetItemList();
}

void CenctrlAbilitySlice::SetItemList()
{
    UIViewGroup* view = new UIViewGroup();
    view->SetPosition(ZERO_POSITION, ZERO_POSITION, SWIPE_VIEW_W, ITEM_LIST_HEIGHT);
    view->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    rootview_->Add(view);
    itemView_ = view;

    UILabel* label = new UILabel();
    label->SetPosition(300, 5, 400, 140);
    label->SetAlign(TEXT_ALIGNMENT_LEFT, TEXT_ALIGNMENT_TOP);
    label->SetText("客厅中控面板");
    label->SetFont(FOND_PATH, 50);
    label->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    label->SetStyle(STYLE_BORDER_RADIUS, LABEL_RADIUS);
    label->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    view->Add(label);
    
    UIImageView* imageView = new UIImageView();
    imageView->SetPosition(BACK_IMAGE_X, BACK_IMAGE_Y, BACK_IMAGE_W, BACK_IMAGE_H);
    imageView->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    imageView->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::Black()));
    imageView->SetSrc(RES_BACK);
    imageView->SetTouchable(true);
    imageView->SetViewId("back");


    
    auto toLauncher = [this](UIView &view, const Event &event) -> bool {
        TerminateAbility();
        return true;
    };

    EventListener* buttonBackListener = new EventListener(toLauncher, nullptr);
    listeners_.PushBack(buttonBackListener);
    imageView->SetOnClickListener(buttonBackListener);
    view->Add(imageView);
}


void CenctrlAbilitySlice::SetUpSwipeView()
{
    const int animatorTime = 20;
    swipeView_ = new UISwipeView();
    swipeView_->SetPosition(0, 70, SWIPE_VIEW_W, SWIPE_VIEW_H);
    swipeView_->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    swipeView_->SetLoopState(true);
    swipeView_->SetAnimatorTime(animatorTime);

    rootview_->Add(swipeView_);
    SetUpFirstView();
    swipeView_->SetCurrentPage(0);
    swipeView_->Invalidate();
}

void CenctrlAbilitySlice::SetUpItem(std::string discrible, std::string iconDir, GridLayout* layout,
                                    std::string devName, std::string changeIconDir)
{
    UIViewGroup* view = new UIViewGroup();
    view->Resize(ITEM_VIEW_W, ITEM_VIEW_H);
    view->SetStyle(STYLE_BACKGROUND_OPA, HALF_OPACITY);
    view->SetStyle(STYLE_BORDER_RADIUS, GROUP_VIEW_RADIUS);
    view->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::Gray()));
    view->SetTouchable(true);
    layout->Add(view);

    UILabelButton* button = new UILabelButton();
    button->SetPosition(ZERO_POSITION, ZERO_POSITION, view->GetWidth(), view->GetHeight());
    button->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    button->SetStyle(STYLE_BORDER_RADIUS, BUTTON_RADIUS);
    button->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_RADIUS, UIButton::PRESSED);
    button->SetStyleForState(STYLE_BACKGROUND_OPA, UN_OPACITY, UIButton::PRESSED);
    view->Add(button);

    UIImageView* imageView = new UIImageView();
    imageView->SetPosition(ITEM_IMAGE_X, ITEM_IMAGE_Y, ITEM_IMAGE_W, ITEM_IMAGE_H);
    imageView->SetSrc(iconDir.c_str());
    view->Add(imageView);

    UILabel* labelFont = new UILabel();
    labelFont->SetPosition(BLANK_W, LABEL_IMAGE_Y, LABEL_IMAGE_W, LABEL_IMAGE_H);
    labelFont->SetText(discrible.c_str());
    labelFont->SetFont(FOND_PATH, BIGBIG_FOND_ID);
    labelFont->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::Gray()));
    labelFont->SetAlign(TEXT_ALIGNMENT_CENTER, TEXT_ALIGNMENT_CENTER);
    labelFont->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    view->Add(labelFont);

    ItemCtrl* item = new ItemCtrl();
    item->Setup(view, labelFont, imageView, iconDir, changeIconDir, devName);
    itemListeners_.PushBack(item);
    button->SetOnClickListener(item);
}

void CenctrlAbilitySlice::SetUpFirstViewLabel(UIViewGroup* view)
{
    UIImageView* imageView1 = new UIImageView();
    imageView1->SetPosition(FIRST_VIEW_IMAGE_X, FIRST_VIEW_IMAGE1_Y, FIRST_VIEW_IMAGE_W, FIRST_VIEW_IMAGE_H);
    imageView1->SetSrc(RES_AIR_TEMP);
    view->Add(imageView1);

    UILabel* label = new UILabel();
    label->SetPosition(FIRST_VIEW_LB2_X, FIRST_VIEW_LB1_Y, FIRST_VIEW_LB2_W, FIRST_VIEW_LB2_H);
    label->SetAlign(TEXT_ALIGNMENT_CENTER, TEXT_ALIGNMENT_CENTER);
    label->SetText("环境温度    10 °C");
    label->SetFont(FOND_PATH, BIG_FOND_ID);
    label->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    label->SetStyle(STYLE_BORDER_RADIUS, LABEL_RADIUS);
    label->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    view->Add(label);

    UIImageView* imageView2 = new UIImageView();
    imageView2->SetPosition(FIRST_VIEW_IMAGE_X, FIRST_VIEW_IMAGE2_Y, FIRST_VIEW_IMAGE_W, FIRST_VIEW_IMAGE_H);
    imageView2->SetSrc(RES_AIR_LUX);
    view->Add(imageView2);
    
    UILabel* label1 = new UILabel();
    label1->SetPosition(FIRST_VIEW_LB2_X, FIRST_VIEW_LB2_Y, FIRST_VIEW_LB2_W, FIRST_VIEW_LB2_H);
    label1->SetAlign(TEXT_ALIGNMENT_CENTER, TEXT_ALIGNMENT_CENTER);
    label1->SetText("光照强度   150 LUX");
    label1->SetFont(FOND_PATH, BIG_FOND_ID);
    label1->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    label1->SetStyle(STYLE_BORDER_RADIUS, LABEL_RADIUS);
    label1->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    view->Add(label1);

    UIImageView* imageView3 = new UIImageView();
    imageView3->SetPosition(FIRST_VIEW_IMAGE_X, FIRST_VIEW_IMAGE3_Y, FIRST_VIEW_IMAGE_W, FIRST_VIEW_IMAGE_H);
    imageView3->SetSrc(RES_AIR_HUM);
    view->Add(imageView3);

    UILabel* label2 = new UILabel();
    label2->SetPosition(FIRST_VIEW_LB2_X, FIRST_VIEW_LB3_Y, FIRST_VIEW_LB2_W, FIRST_VIEW_LB2_H);
    label2->SetAlign(TEXT_ALIGNMENT_CENTER, TEXT_ALIGNMENT_CENTER);
    label2->SetText("环境湿度   50 %");
    label2->SetFont(FOND_PATH, BIG_FOND_ID);
    label2->SetStyle(STYLE_TEXT_COLOR, Color::ColorTo32(Color::White()));
    label2->SetStyle(STYLE_BORDER_RADIUS, LABEL_RADIUS);
    label2->SetStyle(STYLE_BACKGROUND_OPA, TOTAL_OPACITY);
    view->Add(label2);
    g_labelLux_ = label1;
    g_labelHum_ = label2;
    g_labelTemp_ = label;
}

void CenctrlAbilitySlice::SetUpFirstView()
{
    UIViewGroup* lightView = new UIViewGroup();
    lightView->SetPosition(ZERO_POSITION, ZERO_POSITION, SWIPE_VIEW_W, SWIPE_VIEW_H);
    lightView->SetStyle(STYLE_BACKGROUND_OPA, UN_OPACITY);
    lightView->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::Black()));
    swipeView_->Add(lightView);

    UIViewGroup* view = new UIViewGroup();
    view->SetPosition(FIRST_VIEW_X, FIRST_VIEW_Y, FIRST_VIEW_W, FIRST_VIEW_H);
    view->SetStyle(STYLE_BORDER_RADIUS, GROUP_VIEW_RADIUS);
    view->SetStyle(STYLE_BACKGROUND_OPA, HALF_OPACITY);
    view->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::Gray()));
    lightView->Add(view);


    SetUpFirstViewLabel(view);

    GridLayout* layout = new GridLayout();
    layout->SetPosition(FIRST_VIEW_LY_X, FIRST_VIEW_LY_Y, FIRST_VIEW_LY_W, FIRST_VIEW_LY_H);
    layout->SetLayoutDirection(LAYOUT_VER);
    layout->SetRows(LAYOUT_ROWS);
    layout->SetCols(LAYOUT_ROWS);
    lightView->Add(layout);

    SetUpItem("主卧空调", RES_AIR_COND_UNSELECT, layout,"DRLIGHT", RES_AIR_COND_SELECT);
    SetUpItem("主卧筒灯", RES_LIVINGROOM_UNSELECT, layout,"LRLIGHT", RES_LIVINGROOM_SELECT);
    layout->LayoutChildren();
    rootview_->Invalidate();
}

int CenctrlAbilitySlice::DisplayeSensor(int temp, int lux, int hum)
{
    char buf1[TMP_BUF_SIZE] = {0};
    char buf2[TMP_BUF_SIZE] = { 0 };
    char buf3[TMP_BUF_SIZE] = { 0 };
    if (sprintf(buf1,  "环境温度:    %d °C", temp) < 0) {
        return -1;
    }
    g_labelTemp_->SetText(static_cast<char *>(buf1));

    if (sprintf(buf2, "光照强度:    %d Lux", lux) < 0) {
        return -1;
    }
    g_labelLux_->SetText(static_cast<char *>(buf2));

    if (sprintf(buf3,  "环境湿度:    %d %%", hum) < 0) {
        return -1;
    }
    g_labelHum_->SetText(static_cast<char *>(buf3));

    g_labelTemp_->Invalidate();
    g_labelLux_->Invalidate();
    g_labelHum_->Invalidate();
    return 0;
}

void CenctrlAbilitySlice::InitServerSocket()
{
    stCallBackParam callParam;
    callParam.DisplayeSensor = this->DisplayeSensor;
    callParam.On = CtrlCallBack::CtrlOn;
    callParam.Off = CtrlCallBack::CtrlOff;
    callParam.OnConnect = CtrlCallBack::CtrlOnConnect;
    callParam.OnDisconnect = CtrlCallBack::CtrlOnDisconnect;
    RegisterSocketCallback(&callParam);
    CtrlCallBack::SetInstancePram(&itemListeners_);
}

void CenctrlAbilitySlice::OnStart(const Want& want)
{
    LOG(CENCTRL_DEBUG, "### cenctrl slice start ####\n");
    AbilitySlice::OnStart(want);
    rootview_ = RootView::GetWindowRootView();
    rootview_->SetPosition(0, 0, ROOTVIEW_W, ROOTVIEW_H);
    rootview_->SetStyle(STYLE_BACKGROUND_COLOR, Color::ColorTo32(Color::Black()));
    SetUpView();  //主页视图
    SetUIContent(rootview_);
    rootview_->Invalidate();
    InitServerSocket();  //初始化Socket
    SocketUdpCreateBC();    
    SocketServiceCreate();  //创建UDP服务器
}

void CenctrlAbilitySlice::OnInactive()
{
    AbilitySlice::OnInactive();
}

void CenctrlAbilitySlice::OnActive(const Want& want)
{
    AbilitySlice::OnActive(want);
}

void CenctrlAbilitySlice::OnBackground()
{
    AbilitySlice::OnBackground();
}

void CenctrlAbilitySlice::OnStop()
{
    AbilitySlice::OnStop();
}
}