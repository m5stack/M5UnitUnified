/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file ui_UnitBase.cpp
  @brief UI for UnitBase
 */

#include "ui_UnitBase.hpp"

UnitUIBase::UnitUIBase(LovyanGFX* parent) : _parent(parent)
{
    _sem = xSemaphoreCreateBinary();
    xSemaphoreGive(_sem);
}

UnitUIBase::~UnitUIBase()
{
    xSemaphoreTake(_sem, portMAX_DELAY);
    vSemaphoreDelete(_sem);
}

bool UnitUIBase::lock(portTickType bt)
{
    return xSemaphoreTake(_sem, bt) == pdTRUE;
}

void UnitUIBase::unlock()
{
    xSemaphoreGive(_sem);
}
