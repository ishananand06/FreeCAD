/***************************************************************************
 *   Copyright (c) 2005 Werner Mayer <wmayer[at]users.sourceforge.net>     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"
#ifndef _PreComp_
# include <cstdlib>
#endif

#include "ColorModel.h"

using namespace App;



ColorField::ColorField ()
  : colorModel(ColorModelTria())
{
    set(ColorModelTria(), -1.0f, 1.0f, 13);
}

ColorField::ColorField (const ColorModel &rclModel, float fMin, float fMax, std::size_t usCt)
  : colorModel(ColorModelTria())
{
    set(rclModel, fMin, fMax, usCt);
}

ColorField::ColorField (const ColorField &rclCF)
  : colorModel(rclCF.colorModel),
    fMin(rclCF.fMin),
    fMax(rclCF.fMax),
    fAscent(rclCF.fAscent),
    fConstant(rclCF.fConstant),
    ctColors(rclCF.ctColors),
    colorField(rclCF.colorField)
{
}

ColorField& ColorField::operator = (const ColorField &rclCF)
{
    colorField = rclCF.colorField;
    return *this;
}

void ColorField::set (const ColorModel &rclModel, float fMin, float fMax, std::size_t usCt)
{
    colorModel = rclModel;
    this->fMin = std::min<float>(fMin, fMax);
    this->fMax = std::max<float>(this->fMin + CCR_EPS, fMax);
    ctColors = std::max<std::size_t>(usCt, colorModel.getCountColors());
    rebuild();
}

void ColorField::setColorModel (const ColorModel &rclModel)
{
    colorModel = rclModel;
    rebuild();
}

void ColorField::rebuild ()
{
    std::size_t usInd1, usInd2, usStep, i;

    colorField.resize(ctColors);


    usStep = std::min<std::size_t>(ctColors / (colorModel.getCountColors() - 1), ctColors - 1);
    usInd1 = 0;
    usInd2 = usStep;
    for (i = 0; i < (colorModel.getCountColors() - 1); i++) {
        interpolate(colorModel.colors[i], usInd1, colorModel.colors[i+1], usInd2);
        usInd1 = usInd2;
        if ((i + 1) == (colorModel.getCountColors() - 2))
            usInd2 = ctColors - 1;
        else
            usInd2 += usStep;
    }

    fAscent   = float(ctColors) / (fMax - fMin);
    fConstant = -fAscent * fMin;
}

// fuellt das Array von Farbe 1, Index 1 bis Farbe 2, Index 2
void ColorField::interpolate (Color clCol1, std::size_t usInd1, Color clCol2, std::size_t usInd2)
{
    std::size_t i;
    float ucR, ucG, ucB;
    float fR, fG, fB, fStep = 1.0f, fLen = float(usInd2 - usInd1);

    colorField[usInd1] = clCol1;
    colorField[usInd2] = clCol2;

    fR = (float(clCol2.r) - float(clCol1.r)) / fLen;
    fG = (float(clCol2.g) - float(clCol1.g)) / fLen;
    fB = (float(clCol2.b) - float(clCol1.b)) / fLen;

    for (i = (usInd1 + 1); i < usInd2; i++) {
        ucR = clCol1.r + fR * fStep;
        ucG = clCol1.g + fG * fStep;
        ucB = clCol1.b + fB * fStep;
        colorField[i] = Color(ucR, ucG, ucB);
        fStep += 1.0f;
    }
}


ColorGradient::ColorGradient ()
:  tColorModel(TRIA),
   tStyle(ZERO_BASED),
   outsideGrayed(false),
   totalModel(ColorModelTria()),
   topModel(ColorModelTriaTop()),
   bottomModel(ColorModelTriaBottom())
{
    setColorModel();
    set(-1.0f, 1.0f, 13, ZERO_BASED, false);
}

ColorGradient::ColorGradient (float fMin, float fMax, std::size_t usCtColors, TStyle tS, bool bOG)
:  tColorModel(TRIA),
   tStyle(tS),
   outsideGrayed(false),
   totalModel(ColorModelTria()),
   topModel(ColorModelTriaTop()),
   bottomModel(ColorModelTriaBottom())
{
    setColorModel();
    set(fMin, fMax, usCtColors, tS, bOG);
}

void ColorGradient::set (float fMin, float fMax, std::size_t usCt, TStyle tS, bool bOG)
{
    _fMin = std::min<float>(fMin, fMax);
    _fMax = std::max<float>(_fMin + CCR_EPS, fMax);
    ctColors = std::max<std::size_t>(usCt, getMinColors());
    tStyle = tS;
    outsideGrayed = bOG;
    rebuild();
}

void ColorGradient::rebuild ()
{
    switch (tStyle)
    {
        case FLOW:
        {
            colorField1.set(totalModel, _fMin, _fMax, ctColors);
            break;
        }
        case ZERO_BASED:
        {
            if ((_fMin < 0.0f) && (_fMax > 0.0f))
            {
                colorField1.set(bottomModel, _fMin, 0.0f, ctColors / 2);
                colorField2.set(topModel, 0.0f, _fMax, ctColors / 2);
            }
            else if (_fMin >= 0.0f)
                colorField1.set(topModel, 0.0f, _fMax, ctColors);
            else
                colorField1.set(bottomModel, _fMin, 0.0f, ctColors);
            break;
        }
    }
}

std::size_t ColorGradient::getMinColors () const
{
    switch (tStyle)
    {
        case FLOW:
            return colorField1.getMinColors();
        case ZERO_BASED:
        {
            if ((_fMin < 0.0f) && (_fMax > 0.0f))
                return colorField1.getMinColors() + colorField2.getMinColors();
            else
                return colorField1.getMinColors();
        }
    }
    return 2;
}

void ColorGradient::setColorModel (TColorModel tModel)
{
    tColorModel = tModel;
    setColorModel();
    rebuild();
}

void ColorGradient::setColorModel ()
{
    switch (tColorModel)
    {
        case TRIA:
        {
            totalModel  = ColorModelTria();
            topModel    = ColorModelTriaTop();
            bottomModel = ColorModelTriaBottom();
            break;
        }
        case INVERSE_TRIA:
        {
            totalModel  = ColorModelInverseTria();
            topModel    = ColorModelInverseTriaTop();
            bottomModel = ColorModelInverseTriaBottom();
            break;
        }
        case GRAY:
        {
            totalModel  = ColorModelGray();
            topModel    = ColorModelGrayTop();
            bottomModel = ColorModelGrayBottom();
            break;
        }
        case INVERSE_GRAY:
        {
            totalModel  = ColorModelInverseGray();
            topModel    = ColorModelInverseGrayTop();
            bottomModel = ColorModelInverseGrayBottom();
            break;
        }
    }

    switch (tStyle)
    {
        case FLOW:
        {
            colorField1.setColorModel(totalModel);
            colorField2.setColorModel(bottomModel);
            break;
        }
        case ZERO_BASED:
        {
            colorField1.setColorModel(topModel);
            colorField2.setColorModel(bottomModel);
            break;
        }
    }
}

ColorLegend::ColorLegend ()
: outsideGrayed(false)
{
    // default  green, red
    colorFields.emplace_back(0, 1, 0);
    colorFields.emplace_back(1, 0, 0);

    names.push_back("Min");
    names.push_back("Max");

    values.push_back(-1.0f);
    values.push_back(0.0f);
    values.push_back(1.0f);
}

ColorLegend::ColorLegend (const ColorLegend &rclCL)
{
    *this = rclCL;
}

ColorLegend& ColorLegend::operator = (const ColorLegend &rclCL)
{
    colorFields = rclCL.colorFields;
    names       = rclCL.names;
    values      = rclCL.values;
    outsideGrayed = rclCL.outsideGrayed;

    return *this;
}

bool ColorLegend::operator == (const ColorLegend &rclCL) const
{
  return (colorFields.size() == rclCL.colorFields.size())                                 &&
         (names.size() == rclCL.names.size())                                             &&
         (values.size() == rclCL.values.size())                                           &&
          std::equal(colorFields.begin(), colorFields.end(), rclCL.colorFields.begin())   &&
          std::equal(names.begin(), names.end(), rclCL.names.begin())                     &&
          std::equal(values.begin(), values.end(), rclCL.values.begin())                  &&
          outsideGrayed == rclCL.outsideGrayed;
}

float ColorLegend::getValue (unsigned long ulPos) const
{
    if (ulPos < values.size())
        return values[ulPos];
    else
        return 0.0f;
}

bool ColorLegend::setValue (unsigned long ulPos, float fVal)
{
    if (ulPos < values.size())
    {
        values[ulPos] = fVal;
        return true;
    }
    else
        return false;
}

Color ColorLegend::getColor (unsigned long ulPos) const
{
    if (ulPos < colorFields.size())
        return colorFields[ulPos];
    else
        return Color();
}

// color as: 0x00rrggbb
uint32_t ColorLegend::getPackedColor (unsigned long ulPos) const
{
    Color clRGB = getColor(ulPos);
    return clRGB.getPackedValue();
}

std::string ColorLegend::getText (unsigned long ulPos) const
{
    if (ulPos < names.size())
        return names[ulPos];
    else
        return "";
}

bool ColorLegend::addMin (const std::string &rclName)
{
    names.push_front(rclName);
    values.push_front(*values.begin() - 1.0f);

    Color clNewRGB;
    clNewRGB.r = ((float)rand() / (float)RAND_MAX);
    clNewRGB.g = ((float)rand() / (float)RAND_MAX);
    clNewRGB.b = ((float)rand() / (float)RAND_MAX);

    colorFields.push_front(clNewRGB);

    return true;
}

bool ColorLegend::addMax (const std::string &rclName)
{
    names.push_back(rclName);
    values.push_back(*(values.end()-1) + 1.0f);

    Color clNewRGB;
    clNewRGB.r = ((float)rand() / (float)RAND_MAX);
    clNewRGB.g = ((float)rand() / (float)RAND_MAX);
    clNewRGB.b = ((float)rand() / (float)RAND_MAX);

    colorFields.push_back(clNewRGB);

    return true;
}

bool ColorLegend::remove (unsigned long ulPos)
{
    if (ulPos < colorFields.size())
    {
        colorFields.erase(colorFields.begin() + ulPos);
        names.erase(names.begin() + ulPos);
        values.erase(values.begin() + ulPos);

        return true;
    }

    return false;
}

void ColorLegend::removeFirst ()
{
    if (colorFields.size() > 0)
    {
        colorFields.erase(colorFields.begin());
        names.erase(names.begin());
        values.erase(values.begin());
    }
}

void ColorLegend::removeLast ()
{
    if (colorFields.size() > 0)
    {
        colorFields.erase(colorFields.end()-1);
        names.erase(names.end()-1);
        values.erase(values.end()-1);
    }
}

void ColorLegend::resize (unsigned long ulCt)
{
    if ((ulCt < 2) || (ulCt == colorFields.size()))
        return;

    if (ulCt > colorFields.size())
    {
        int k = ulCt - colorFields.size();
        for (int i = 0; i < k; i++)
            addMin("new");
    }
    else
    {
        int k = colorFields.size() - ulCt;
        for (int i = 0; i < k; i++)
            removeLast();
    }
}

bool ColorLegend::setColor (unsigned long ulPos, float ucRed, float ucGreen, float ucBlue)
{
    if (ulPos < names.size())
    {
        colorFields[ulPos] = Color(ucRed, ucGreen, ucBlue);
        return true;
    }
    else
        return false;
}

// color as 0x00rrggbb
bool ColorLegend::setColor (unsigned long ulPos, unsigned long ulColor)
{
    unsigned char ucRed   = (unsigned char)((ulColor & 0x00ff0000) >> 16);
    unsigned char ucGreen = (unsigned char)((ulColor & 0x0000ff00) >> 8);
    unsigned char ucBlue  = (unsigned char)(ulColor & 0x000000ff);
    return setColor(ulPos, ucRed, ucGreen, ucBlue);
}

bool ColorLegend::setText (unsigned long ulPos, const std::string &rclName)
{
    if (ulPos < names.size())
    {
        names[ulPos] = rclName;
        return true;
    }
    else
        return false;
}
