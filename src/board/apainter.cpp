/*
 * ARMEmuS pins and leds painter
 *
 * Copyright (c) 2017 ARMEmuS
 *
 * Authors:
 *  Mario Alberto López Ocampo  <malo1986@msn.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#include "apainter.h"

apainter::apainter(QGraphicsView *ptr_view, QGraphicsSvgItem *ptr_svgItem) : m_view(ptr_view) , m_svgItem(ptr_svgItem)
{
    clear();
}

apainter::~apainter()
{
    clear();
}

void apainter::clear()
{
    if(!outputPins.isEmpty()){
        for(int i=0; i<outputPins.count();i++)
            delete outputPins[i];
    }

    if(!Leds.isEmpty()){
        for(int i=0; i<Leds.count();i++)
            delete Leds[i];
    }

    if(!inputPins.isEmpty()){
        for(int i=0; i<inputPins.count();i++)
            delete inputPins[i];
    }

    outputPins.clear();
    Leds.clear();
    inputPins.clear();
    disableInputs.clear();
    inputsState.clear();
}


void apainter::setBoard(const IOpins *ptr_IOpins)
{
    for(int i=0; i<ptr_IOpins->pinsboard.count(); i++){
         QAbstractGraphicsShapeItem *ptr_pin;

         if(ptr_IOpins->pinRect)
             ptr_pin=new QGraphicsRectItem(ptr_IOpins->pinsboard.at(i).x, ptr_IOpins->pinsboard.at(i).y, ptr_IOpins->whpin[0], ptr_IOpins->whpin[1], m_svgItem);
         else
             ptr_pin=new QGraphicsEllipseItem(ptr_IOpins->pinsboard.at(i).x, ptr_IOpins->pinsboard.at(i).y, ptr_IOpins->whpin[0], ptr_IOpins->whpin[1], m_svgItem);

         ptr_pin->setPen(Qt::NoPen);
         ptr_pin->hide();

         outputPins.append(ptr_pin);         
    }

    for(int i=0; i<ptr_IOpins->ledsboard.count(); i++){
         QAbstractGraphicsShapeItem *ptr_led;

         if(ptr_IOpins->ledRect)
             ptr_led=new QGraphicsRectItem(ptr_IOpins->ledsboard.at(i).x, ptr_IOpins->ledsboard.at(i).y, ptr_IOpins->whled[0], ptr_IOpins->whled[1], m_svgItem);
         else
             ptr_led=new QGraphicsEllipseItem(ptr_IOpins->ledsboard.at(i).x, ptr_IOpins->ledsboard.at(i).y, ptr_IOpins->whled[0], ptr_IOpins->whled[1], m_svgItem);

         ptr_led->setBrush(QColor(ptr_IOpins->ledsboard.at(i).color));
         ptr_led->setPen(Qt::NoPen);
         ptr_led->hide();
         Leds.append(ptr_led);
    }

//-------------------------------------------------------//-------------------------------------------------------//    

    QPen lowInput(Qt::blue);

    for(int i=0; i<ptr_IOpins->pinsboard.count(); i++){
         QAbstractGraphicsShapeItem *ptr_input;

         if(ptr_IOpins->pinRect)
             ptr_input=new QGraphicsRectItem(ptr_IOpins->pinsboard.at(i).x, ptr_IOpins->pinsboard.at(i).y, ptr_IOpins->whpin[0], ptr_IOpins->whpin[1], m_svgItem);
         else
             ptr_input=new QGraphicsEllipseItem(ptr_IOpins->pinsboard.at(i).x, ptr_IOpins->pinsboard.at(i).y, ptr_IOpins->whpin[0], ptr_IOpins->whpin[1], m_svgItem);
             //ptr_input=new QGraphicsEllipseItem(ptr_IOpins->pinsboard.at(i).x, ptr_IOpins->pinsboard.at(i).y, ptr_IOpins->whpin[0], ptr_IOpins->whpin[1], m_svgItem);

         ptr_input->setPen(lowInput);
         ptr_input->hide();

         inputPins.append(ptr_input);
         inputsState.append(false);

    }    

//-------------------------------------------------------//-------------------------------------------------------//

}


void apainter::drawPin(const QByteArray &Reader,int index)
{


    if(!outputPins.at(index)->isVisible()){
        outputPins.at(index)->show();
        disableInputs.append(index);
    }

    if(Reader.contains("HIGH"))
        outputPins.at(index)->setBrush(Qt::red);
    else
        outputPins.at(index)->setBrush(Qt::blue);

    inputPins.at(index)->hide();

}

void apainter::drawLed(const QByteArray &Reader,int index)
{
    if(Reader.contains("HIGH"))
        Leds.at(index)->show();
    else
        Leds.at(index)->hide();

    if (index==0)
        for(int i=0; i<inputPins.count();i++)
            inputPins.at(i)->show();
}

void apainter::turnOff(){

    QPen lowInput(Qt::blue);

    for(int i=0; i<outputPins.count();i++)
        outputPins.at(i)->hide();

    for(int i=0; i<Leds.count();i++)
        Leds.at(i)->hide();

    for(int i=0; i<inputPins.count();i++){
        inputPins.at(i)->hide();
        inputPins.at(i)->setPen(lowInput);
        inputsState[i]=false;
    }

    disableInputs.clear();

}

//-------------------------------------------------------//-------------------------------------------------------//

void apainter::inputEvent()
{
    QPen lowInput(Qt::blue);
    QPen highInput(Qt::red);

    for(int i=0;i<inputPins.count();i++){
        if(inputPins.at(i)->isUnderMouse() && !disableInputs.contains(i)){            
            emit printInputpin(i, inputsState[i]=!inputsState.at(i));
            if(inputsState[i])
                inputPins.at(i)->setPen(highInput);
            else
                inputPins.at(i)->setPen(lowInput);

        //=!inputsState.at(i);
        }
    }
}

void apainter::showInput()
{
    for(int i=0; i<inputPins.count();i++)
        if(inputPins.at(i)->isUnderMouse() && !disableInputs.contains(i))
                m_view->setCursor(Qt::PointingHandCursor);
}
