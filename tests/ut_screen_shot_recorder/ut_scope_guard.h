// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// RAII guard that restores a global flag on destruction. Ensures the flag is
// restored even if the test fails or throws, keeping tests isolated.
template <typename T>
class UtScopeGuard
{
public:
    UtScopeGuard() = default;
    UtScopeGuard(const UtScopeGuard &) = delete;
    UtScopeGuard &operator=(const UtScopeGuard &) = delete;

    void protect(T &flag, const T &value)
    {
        restore();
        m_ptr = &flag;
        m_prev = flag;
        flag = value;
    }

    void restore()
    {
        if (m_ptr) {
            *m_ptr = m_prev;
            m_ptr = nullptr;
        }
    }

    ~UtScopeGuard() { restore(); }

private:
    T *m_ptr = nullptr;
    T m_prev {};
};
