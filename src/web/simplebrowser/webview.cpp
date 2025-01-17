/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "browserwindow.h"
#include "tabwidget.h"
#include "webpage.h"
#include "webpopupwindow.h"
#include "webview.h"
#include "WBHistory.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QWebEngineContextMenuData>
#include <QWebEngineProfile>

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent)
    , m_loadProgress(100)
{
    connect(this, &QWebEngineView::loadStarted, [this]() {
        m_loadProgress = 0;
        emit favIconChanged(favIcon());
    });
    connect(this, &QWebEngineView::loadProgress, [this](int progress) {
        m_loadProgress = progress;
    });
    connect(this, &QWebEngineView::loadFinished, [this](bool success) {
        m_loadProgress = success ? 100 : -1;
        emit favIconChanged(favIcon());
    });
    connect(this, &QWebEngineView::iconChanged, [this](const QIcon &) {
        emit favIconChanged(favIcon());
    });
    connect(this, &QWebEngineView::urlChanged, [this](const QUrl& url){
        BrowserWindow* browser = browserWindow();

        if (browser && page() && !page()->profile()->isOffTheRecord()) {
            browser->historyManager()->addHistoryEntry(url);
        }
    });

    connect(this, &QWebEngineView::renderProcessTerminated,
            [this](QWebEnginePage::RenderProcessTerminationStatus termStatus, int statusCode) {
        QString status;
        switch (termStatus) {
        case QWebEnginePage::NormalTerminationStatus:
            status = tr("Render process normal exit");
            break;
        case QWebEnginePage::AbnormalTerminationStatus:
            status = tr("Render process abnormal exit");
            break;
        case QWebEnginePage::CrashedTerminationStatus:
            status = tr("Render process crashed");
            break;
        case QWebEnginePage::KilledTerminationStatus:
            status = tr("Render process killed");
            break;
        }
        QMessageBox::StandardButton btn = QMessageBox::question(window(), status,
                                                   tr("Render process exited with code: %1\n"
                                                      "Do you want to reload the page ?").arg(statusCode));
        if (btn == QMessageBox::Yes)
            QTimer::singleShot(0, [this] { reload(); });
    });
}

void WebView::setPage(WebPage *page)
{
    createWebActionTrigger(page,QWebEnginePage::Forward);
    createWebActionTrigger(page,QWebEnginePage::Back);
    createWebActionTrigger(page,QWebEnginePage::Reload);
    createWebActionTrigger(page,QWebEnginePage::Stop);
    QWebEngineView::setPage(page);
}

int WebView::loadProgress() const
{
    return m_loadProgress;
}

void WebView::createWebActionTrigger(QWebEnginePage *page, QWebEnginePage::WebAction webAction)
{
    QAction *action = page->action(webAction);
    connect(action, &QAction::changed, [this, action, webAction]{
        emit webActionEnabledChanged(webAction, action->isEnabled());
    });
}

void WebView::openExternalBrowser(const QUrl &url)
{
    // open url in system default browser
    QDesktopServices::openUrl(url);

    // delete temporary view
    parent()->deleteLater();
}

BrowserWindow *WebView::browserWindow()
{
    QWidget *w = this;
    QWidget *p = w->parentWidget();
    BrowserWindow* browserWindow;

    while (!(browserWindow = qobject_cast<BrowserWindow*>(w)) && p) {
        w = p;
        p = p->parentWidget();
    }

    return browserWindow;
}

bool WebView::isWebActionEnabled(QWebEnginePage::WebAction webAction) const
{
    return page()->action(webAction)->isEnabled();
}

QIcon WebView::favIcon() const
{
    QIcon favIcon = icon();
    if (!favIcon.isNull())
        return favIcon;

    if (m_loadProgress < 0) {
        static QIcon errorIcon(QStringLiteral(":dialog-error.png"));
        return errorIcon;
    } else if (m_loadProgress < 100) {
        static QIcon loadingIcon(QStringLiteral(":view-refresh.png"));
        return loadingIcon;
    } else {
        static QIcon defaultIcon(QStringLiteral(":text-html.png"));
        return defaultIcon;
    }
}

QWebEngineView *WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    BrowserWindow* browser = browserWindow();

    if (!browser)
        return nullptr;

    switch (type) {
    case QWebEnginePage::WebBrowserWindow: {
        // create a hidden, temporary view to catch the urlChanged signal
        WebPopupWindow *popup = new WebPopupWindow(page()->profile());
        popup->hide();
        connect(popup->view(), &QWebEngineView::urlChanged, popup->view(), &WebView::openExternalBrowser);
        return popup->view();
    }
    case QWebEnginePage::WebBrowserTab: {
        return browser->tabWidget()->createTab();
    }
    case QWebEnginePage::WebBrowserBackgroundTab: {
        return browser->tabWidget()->createBackgroundTab();
    }
    case QWebEnginePage::WebDialog: {
        WebPopupWindow *popup = new WebPopupWindow(page()->profile());
        connect(popup->view(), &WebView::devToolsRequested, this, &WebView::devToolsRequested);
        return popup->view();
    }
    }
    return nullptr;
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = page()->createStandardContextMenu();
    const QList<QAction *> actions = menu->actions();
    auto inspectElement = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::InspectElement));
    if (inspectElement == actions.cend()) {
        auto viewSource = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::ViewSource));
        if (viewSource == actions.cend())
            menu->addSeparator();

        QAction *action = new QAction(menu);
        action->setText(tr("Open Web Inspector in new window"));
        connect(action, &QAction::triggered, [this]() { emit devToolsRequested(page()); });

        QAction *before(inspectElement == actions.cend() ? nullptr : *inspectElement);
        menu->insertAction(before, action);
    } else {
        (*inspectElement)->setText(tr("Inspect element"));
    }

    // add menu entry "Add to board" with PDF browser, PDF link or image
    QUrl contentUrl;

    QUrl pageUrl = page()->url();
    QUrl linkUrl = page()->contextMenuData().linkUrl();
    auto mediaType = page()->contextMenuData().mediaType();

    if (pageUrl.scheme() == "chrome-extension" && pageUrl.query().endsWith(".pdf", Qt::CaseInsensitive))
    {
        // in PDF browser, browsed document URL is available in the query
        contentUrl = pageUrl.query();
    }
    else if (linkUrl.path().endsWith(".pdf", Qt::CaseInsensitive))
    {
        // on PDF link
        contentUrl = linkUrl;
    }
    else if (mediaType == QWebEngineContextMenuData::MediaTypeImage)
    {
        // on image
        contentUrl = page()->contextMenuData().mediaUrl();
    }

    if (contentUrl.isValid())
    {
        QAction* actionAddToBoard = new QAction(menu);
        actionAddToBoard->setText(tr("Add to board"));
        connect(actionAddToBoard, &QAction::triggered, [contentUrl](){
            UBApplication::applicationController->showBoard();
            UBApplication::boardController->downloadURL(contentUrl);
        });

        menu->addSeparator();
        menu->addAction(actionAddToBoard);
    }

    menu->popup(event->globalPos());
}

