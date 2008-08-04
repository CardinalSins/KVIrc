//=============================================================================
//
//   File : kvi_tal_wizard.cpp
//   Creation date : Tue Feb 06 2007 14:35:08 by Szymon Stefanek
//
//   This file is part of the KVirc irc client distribution
//   Copyright (C) 2007 Szymon Stefanek (pragma at kvirc dot net)
//
//   This program is FREE software. You can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your opinion) any later version.
//
//   This program is distributed in the HOPE that it will be USEFUL,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program. If not, write to the Free Software Foundation,
//   Inc. ,59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//=============================================================================

#define __KVILIB__
#include "kvi_tal_wizard.h"
#include "kvi_tal_hbox.h"
#include "kvi_pointerlist.h"
#include "kvi_locale.h"

#ifdef COMPILE_USE_QT4
	#include <QShowEvent>
	#include <QLabel>
	#include <QPushButton>
	#include <QGridLayout>
	#include <QFrame>
	#include <QStackedWidget>
	#include <QPalette>
#else
	#include <qlabel.h>
	#include <qpushbutton.h>
	#include <qlayout.h>
	#include <qevent.h>
	#include <qframe.h>
	#include <qwidgetstack.h>
	#include <qpalette.h>
#endif


class KviTalWizardPageData
{
public:
	enum EnableFlags
	{
		EnableNext = 1,
		EnableBack = 2,
		EnableHelp = 4,
		EnableCancel = 8,
		EnableFinish = 16
	};
public:
	QWidget * pWidget;
	QString   szTitle;
	bool      bEnabled;
	int       iEnableFlags;
	int       iVisibleIndex;
};

class KviTalWizardPrivate
{
public:
	KviPointerList<KviTalWizardPageData> * pPageList;
	int                                iEnabledPageCount;
	KviTalWizardPageData             * pCurrentPage;
	QGridLayout                      * pLayout;
	QLabel                           * pTitleLabel;
	QLabel                           * pStepsLabel;
	QPushButton                      * pBackButton;
	QPushButton                      * pCancelButton;
	QPushButton                      * pHelpButton;
	QPushButton                      * pNextButton;
	QWidget                          * pNextSpacer;
	QPushButton                      * pFinishButton;
	QWidget                          * pFinishSpacer;
#ifdef COMPILE_USE_QT4
	QStackedWidget                   * pWidgetStack;
#else
	QWidgetStack                     * pWidgetStack;
#endif
public:
	KviTalWizardPageData * findPage(QWidget * pWidget)
	{
		for(KviTalWizardPageData * pData = pPageList->first();pData;pData = pPageList->next())
		{
			if(pData->pWidget == pWidget)
				return pData;
		}
		return NULL;
	}

	KviTalWizardPageData * findFirstEnabledPage()
	{
		KviTalWizardPageData * pData;
		for(pData = pPageList->first();pData;pData = pPageList->next())
		{
			if(pData->bEnabled)
				return pData;
		}
		return NULL;
	}

	KviTalWizardPageData * findLastEnabledPage()
	{
		KviTalWizardPageData * pData;
		for(pData = pPageList->last();pData;pData = pPageList->prev())
		{
			if(pData->bEnabled)
				return pData;
		}
		return NULL;
	}

	KviTalWizardPageData * findNextEnabledPage(QWidget * pReference)
	{
		if(!pReference)
			return findFirstEnabledPage();
		KviTalWizardPageData * pData = findPage(pReference);
		if(!pData)
			return NULL;
		for(pData = pPageList->next();pData;pData = pPageList->next())
		{
			if(pData->bEnabled)
				return pData;
		}
		return NULL;
	}

	KviTalWizardPageData * findPrevEnabledPage(QWidget * pReference)
	{
		if(!pReference)
			return findLastEnabledPage();
		KviTalWizardPageData * pData = findPage(pReference);
		if(!pData)
			return NULL;
		for(pData = pPageList->prev();pData;pData = pPageList->prev())
		{
			if(pData->bEnabled)
				return pData;
		}
		return NULL;
	}
	
	int reindexPages()
	{
		int iEnabledCount = 0;
		for(KviTalWizardPageData * pData = pPageList->next();pData;pData = pPageList->next())
		{
			if(pData->bEnabled)
			{
				iEnabledCount++;
				pData->iVisibleIndex = iEnabledCount;
			}
		}
		return iEnabledCount;
	}
};


KviTalWizard::KviTalWizard(QWidget * pParent)
: QDialog(pParent)
{
	m_p = new KviTalWizardPrivate;
	m_p->pPageList = new KviPointerList<KviTalWizardPageData>;
	m_p->pPageList->setAutoDelete(true);
	m_p->pCurrentPage = NULL;
	m_p->iEnabledPageCount = 0;
	m_p->pLayout = new QGridLayout(this);

	m_p->pTitleLabel = new QLabel(this);
#ifdef COMPILE_USE_QT4
	m_p->pLayout->addWidget(m_p->pTitleLabel,0,0,1,3);
#else
	m_p->pLayout->addMultiCellWidget(m_p->pTitleLabel,0,0,0,3);
#endif
	m_p->pStepsLabel = new QLabel(this);
	m_p->pStepsLabel->setMinimumWidth(80);
	m_p->pStepsLabel->setAlignment(Qt::AlignRight);
#ifdef COMPILE_USE_QT4
	m_p->pLayout->addWidget(m_p->pStepsLabel,0,4,1,3);
#else
	m_p->pLayout->addMultiCellWidget(m_p->pStepsLabel,0,0,4,6);
#endif

	QFrame * f1 = new QFrame(this);
	f1->setFrameStyle(QFrame::Sunken | QFrame::HLine);
#ifdef COMPILE_USE_QT4
	m_p->pLayout->addWidget(f1,1,0,1,7);
#else
	m_p->pLayout->addMultiCellWidget(f1,1,1,0,6);
#endif

#ifdef COMPILE_USE_QT4
	m_p->pWidgetStack = new QStackedWidget(this);
	m_p->pLayout->addWidget(m_p->pWidgetStack,2,0,1,7);
#else
	m_p->pWidgetStack = new QWidgetStack(this);
	m_p->pLayout->addMultiCellWidget(m_p->pWidgetStack,2,2,0,6);
#endif

	QFrame * f2 = new QFrame(this);
	f2->setFrameStyle(QFrame::Sunken | QFrame::HLine);
#ifdef COMPILE_USE_QT4
	m_p->pLayout->addWidget(f2,3,0,1,7);
#else
	m_p->pLayout->addMultiCellWidget(f2,3,3,0,6);
#endif

	KviTalHBox * pButtonBox = new KviTalHBox(this);
#ifdef COMPILE_USE_QT4
	m_p->pLayout->addWidget(pButtonBox,4,0,1,7);
#else
	m_p->pLayout->addMultiCellWidget(pButtonBox,4,4,0,6);
#endif

	pButtonBox->setMargin(0);
	pButtonBox->setSpacing(0);


	m_p->pCancelButton = new QPushButton(__tr("Cancel"),pButtonBox);
	m_p->pCancelButton->setMinimumWidth(80);
	QObject::connect(
			m_p->pCancelButton,
			SIGNAL(clicked()),
			this,
			SLOT(cancelButtonClicked())
		);

	QWidget * pSpacer = new QWidget(pButtonBox);
	pSpacer->setFixedWidth(4);

	m_p->pHelpButton = new QPushButton(__tr("Help"),pButtonBox);
	m_p->pHelpButton->setMinimumWidth(80);
	QObject::connect(
			m_p->pHelpButton,
			SIGNAL(clicked()),
			this,
			SLOT(helpButtonClicked())
		);

	QWidget * pLargeSpacer = new QWidget(pButtonBox);
	pLargeSpacer->setMinimumWidth(50);
	pButtonBox->setStretchFactor(pLargeSpacer,100);

	QString szText = "< ";
	szText += __tr("Back");
	m_p->pBackButton = new QPushButton(szText,pButtonBox);
	m_p->pBackButton->setMinimumWidth(80);
	QObject::connect(
			m_p->pBackButton,
			SIGNAL(clicked()),
			this,
			SLOT(backButtonClicked())
		);

	m_p->pNextSpacer = new QWidget(pButtonBox);
	m_p->pNextSpacer->setFixedWidth(4);

	szText = __tr("Next");
	szText += " >";
	m_p->pNextButton = new QPushButton(szText,pButtonBox);
	m_p->pNextButton->setMinimumWidth(80);
	QObject::connect(
			m_p->pNextButton,
			SIGNAL(clicked()),
			this,
			SLOT(nextButtonClicked())
		);

	m_p->pFinishSpacer = new QWidget(pButtonBox);
	m_p->pFinishSpacer->setFixedWidth(4);

	m_p->pFinishButton = new QPushButton(__tr("Finish"),pButtonBox);
	m_p->pFinishButton->setMinimumWidth(80);
	QObject::connect(
			m_p->pFinishButton,
			SIGNAL(clicked()),
			this,
			SLOT(finishButtonClicked())
		);
		
	m_p->pLayout->setMargin(8);
	m_p->pLayout->setSpacing(4);
	m_p->pLayout->setRowStretch(2,1);
	m_p->pLayout->setColStretch(0,1);
}

KviTalWizard::~KviTalWizard()
{
	delete m_p->pPageList;
	delete m_p;
}

void KviTalWizard::insertPage(QWidget * pWidget,const QString &szTitle,int iIndex)
{
	KviTalWizardPageData * pPageData = m_p->findPage(pWidget);
	if(!pPageData)
	{
		pPageData = new KviTalWizardPageData;
		pPageData->pWidget = pWidget;
		pPageData->iEnableFlags = \
			KviTalWizardPageData::EnableNext | \
			KviTalWizardPageData::EnableCancel | \
			KviTalWizardPageData::EnableBack;
		if(iIndex < 0)
		{
			m_p->pPageList->append(pPageData);
			m_p->iEnabledPageCount++;
			pPageData->iVisibleIndex = m_p->iEnabledPageCount;
		} else {
			m_p->pPageList->insert(iIndex,pPageData);
			m_p->iEnabledPageCount = m_p->reindexPages();
		}
		m_p->pWidgetStack->addWidget(pWidget);
	}
	pPageData->szTitle = szTitle;
	pPageData->bEnabled = true;

}

void KviTalWizard::addPage(QWidget * pWidget,const QString &szTitle)
{
	insertPage(pWidget,szTitle,-1);
}

bool KviTalWizard::setPageEnabled(QWidget * pWidget,bool bEnabled)
{
	KviTalWizardPageData * pData = m_p->findPage(pWidget);
	if(!pData)
		return false;
	pData->bEnabled = bEnabled;
	m_p->iEnabledPageCount = m_p->reindexPages();
	setCurrentPage(m_p->pCurrentPage);
	return true;
}

bool KviTalWizard::setPageTitle(QWidget * pWidget,const QString &szTitle)
{
	KviTalWizardPageData * pData = m_p->findPage(pWidget);
	if(!pData)
		return false;
	pData->szTitle = szTitle;
	return true;
}

bool KviTalWizard::setCurrentPage(QWidget * pWidget)
{
	KviTalWizardPageData * pData = m_p->findPage(pWidget);
	if(!pData)
		return false;
	setCurrentPage(pData);
	return true;
}

QWidget * KviTalWizard::currentPage()
{
	if(!m_p->pCurrentPage)
		return NULL;
	return m_p->pCurrentPage->pWidget;
}

void KviTalWizard::setCurrentPage(KviTalWizardPageData * pData)
{
	m_p->pCurrentPage = pData;

	bool bCancelEnabled = true;
	bool bNextEnabled = false;
	bool bBackEnabled = false;
	bool bHelpEnabled = false;
	bool bFinishEnabled = false;

	QString szTitle;
	QString szSteps;

	if(pData)
	{
		bNextEnabled = (pData->iEnableFlags & KviTalWizardPageData::EnableNext) && m_p->findNextEnabledPage(pData->pWidget);
		bBackEnabled = (pData->iEnableFlags & KviTalWizardPageData::EnableBack) && m_p->findPrevEnabledPage(pData->pWidget);
		bCancelEnabled = (pData->iEnableFlags & KviTalWizardPageData::EnableCancel);
		bFinishEnabled = (pData->iEnableFlags & KviTalWizardPageData::EnableFinish);
		bHelpEnabled = (pData->iEnableFlags & KviTalWizardPageData::EnableHelp);
#ifdef COMPILE_USE_QT4
		m_p->pWidgetStack->setCurrentWidget(pData->pWidget);
#else
		m_p->pWidgetStack->raiseWidget(pData->pWidget);
#endif
		szTitle = "<b>";
		szTitle += pData->szTitle;
		szTitle += "</b>";
		QPalette pal = m_p->pStepsLabel->palette();
#ifdef COMPILE_USE_QT4
		QColor clrWin = pal.color(QPalette::Normal,QPalette::Window);
		QColor clrTxt = pal.color(QPalette::Normal,QPalette::WindowText);
#else
		QColor clrWin = pal.color(QPalette::Normal,QColorGroup::Foreground);
		QColor clrTxt = pal.color(QPalette::Normal,QColorGroup::Background);
#endif
		QColor clrMid = qRgb(
							(clrWin.red() + clrTxt.red()) / 2,
							(clrWin.green() + clrTxt.green()) / 2,
							(clrWin.blue() + clrTxt.blue()) / 2
						);
		
		szSteps = "<nobr><font color=\"";
		szSteps += clrMid.name();
		szSteps += "\"><b>[";
		szSteps += QString("Step %1 of %2").arg(pData->iVisibleIndex).arg(m_p->iEnabledPageCount);
		szSteps += "]</b></font></nobr>";
	}

	m_p->pTitleLabel->setText(szTitle);
	m_p->pStepsLabel->setText(szSteps);

	m_p->pNextButton->setEnabled(bNextEnabled);
	if(bNextEnabled)
	{
		m_p->pNextButton->show();
		m_p->pNextSpacer->show();
	} else {
		m_p->pNextButton->hide();
		m_p->pNextSpacer->hide();
	}
	m_p->pBackButton->setEnabled(bBackEnabled);
	m_p->pHelpButton->setEnabled(bHelpEnabled);
	if(bHelpEnabled)
		m_p->pHelpButton->show();
	else
		m_p->pHelpButton->hide();
	m_p->pCancelButton->setEnabled(bCancelEnabled);
	m_p->pFinishButton->setEnabled(bFinishEnabled);
	if(bFinishEnabled)
	{
		m_p->pFinishButton->show();
		m_p->pFinishSpacer->show();
	} else {
		m_p->pFinishButton->hide();
		m_p->pFinishSpacer->hide();
	}
}

void KviTalWizard::showEvent(QShowEvent * e)
{
	if(!(m_p->pCurrentPage))
	{
		// display the first page
		KviTalWizardPageData * pData = m_p->findFirstEnabledPage();
		if(pData)
			setCurrentPage(pData->pWidget);
	}
	QDialog::showEvent(e);
	emit pageChanged(m_p->pCurrentPage->szTitle);
}

void KviTalWizard::closeEvent(QCloseEvent * e)
{
	e->ignore();
	cancelButtonClicked();
}

void KviTalWizard::backButtonClicked()
{
	if(!m_p->pCurrentPage)
		return;
	setCurrentPage(m_p->findPrevEnabledPage(m_p->pCurrentPage->pWidget));
	emit pageChanged(m_p->pCurrentPage->szTitle);
}

void KviTalWizard::nextButtonClicked()
{
	setCurrentPage(m_p->findNextEnabledPage(m_p->pCurrentPage->pWidget));
	emit pageChanged(m_p->pCurrentPage->szTitle);
}

void KviTalWizard::helpButtonClicked()
{
	emit helpClicked();
}

void KviTalWizard::cancelButtonClicked()
{
	reject();
}

void KviTalWizard::finishButtonClicked()
{
	accept();
}

void KviTalWizard::setHelpEnabled(QWidget * pWidget,bool bEnabled)
{
	KviTalWizardPageData * pData = m_p->findPage(pWidget);
	if(!pData)
		return;
	if(bEnabled)
		pData->iEnableFlags |= KviTalWizardPageData::EnableHelp;
	else
		pData->iEnableFlags &= ~KviTalWizardPageData::EnableHelp;
	if(pData == m_p->pCurrentPage)
		setCurrentPage(pData);
}

void KviTalWizard::setCancelEnabled(QWidget * pWidget,bool bEnabled)
{
	KviTalWizardPageData * pData = m_p->findPage(pWidget);
	if(!pData)
		return;
	if(bEnabled)
		pData->iEnableFlags |= KviTalWizardPageData::EnableCancel;
	else
		pData->iEnableFlags &= ~KviTalWizardPageData::EnableCancel;
	if(pData == m_p->pCurrentPage)
		setCurrentPage(pData);
}

void KviTalWizard::setFinishEnabled(QWidget * pWidget,bool bEnabled)
{
	KviTalWizardPageData * pData = m_p->findPage(pWidget);
	if(!pData)
		return;
	if(bEnabled)
		pData->iEnableFlags |= KviTalWizardPageData::EnableFinish;
	else
		pData->iEnableFlags &= ~KviTalWizardPageData::EnableFinish;
	if(pData == m_p->pCurrentPage)
		setCurrentPage(pData);
}

void KviTalWizard::setNextEnabled(QWidget * pWidget,bool bEnabled)
{
	KviTalWizardPageData * pData = m_p->findPage(pWidget);
	if(!pData)
		return;
	if(bEnabled)
		pData->iEnableFlags |= KviTalWizardPageData::EnableNext;
	else
		pData->iEnableFlags &= ~KviTalWizardPageData::EnableNext;
	if(pData == m_p->pCurrentPage)
		setCurrentPage(pData);
}

void KviTalWizard::setBackEnabled(QWidget * pWidget,bool bEnabled)
{
	KviTalWizardPageData * pData = m_p->findPage(pWidget);
	if(!pData)
		return;
	if(bEnabled)
		pData->iEnableFlags |= KviTalWizardPageData::EnableBack;
	else
		pData->iEnableFlags &= ~KviTalWizardPageData::EnableBack;
	if(pData == m_p->pCurrentPage)
		setCurrentPage(pData);
}

QPushButton * KviTalWizard::cancelButton()
{
	return m_p->pCancelButton;
}

QPushButton * KviTalWizard::helpButton()
{
	return m_p->pHelpButton;
}

QPushButton * KviTalWizard::finishButton()
{
	return m_p->pFinishButton;
}

QPushButton * KviTalWizard::nextButton()
{
	return m_p->pNextButton;
}

QPushButton * KviTalWizard::backButton()
{
	return m_p->pBackButton;
}

