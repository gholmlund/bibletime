/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2018 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License
* version 2.0.
*
**********/

#include "BookmarkItem.h"


BookmarkItem::BookmarkItem(QString const & module,
                           QString const & key,
                           QString const & description)
    : m_moduleName(module)
    , m_key(key)
    , m_description(description)
{}
