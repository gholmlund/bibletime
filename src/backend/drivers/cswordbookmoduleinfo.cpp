/*********
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2016 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#include "cswordbookmoduleinfo.h"

#include <swordxx/keys/treekey.h>
#include <swordxx/keys/treekeyidx.h>
#include "../../util/btassert.h"
#include "../keys/cswordtreekey.h"


CSwordBookModuleInfo::CSwordBookModuleInfo(swordxx::SWModule & module,
                                           CSwordBackend & backend)
    : CSwordModuleInfo(module, backend, CSwordModuleInfo::GenericBook)
    , m_depth(-1)
{
    swordxx::TreeKeyIdx *key = tree();
    if (key) {
        key->root();
        computeDepth(key, 0);
    }
}

void CSwordBookModuleInfo::computeDepth(swordxx::TreeKeyIdx * const key,
                                        const int level)
{
    const QByteArray savedKey(key->getText());
    if (level > m_depth)
        m_depth = level;

    if (key->hasChildren()) {
        key->firstChild();
        computeDepth(key, level + 1u);
        key->setText(savedKey.constData()); // Return to the initial value
    }

    if (key->nextSibling())
        computeDepth(key, level);
}

/** Returns a treekey filled with the structure of this module */
swordxx::TreeKeyIdx* CSwordBookModuleInfo::tree() const {
    swordxx::TreeKeyIdx * const treeKey =
            dynamic_cast<swordxx::TreeKeyIdx *>(module().getKey());
    BT_ASSERT(treeKey);
    return treeKey;
}
