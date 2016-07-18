/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2016 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#ifndef BTSTRINGMGR_H
#define BTSTRINGMGR_H

#include <swordxx/stringmgr.h>


/**
  Unicode string manager implementation.

  A Qt-based swordxx::StringMgr is better than the default one in Sword++, in case
  Sword++ is not compiled against ICU regarding this. However, we have no good
  means to check this at compile time, hence we provide this class. It is
  initialized in BibleTime::initBackends() as follows:
  \code{.cpp}
  if (!swordxx::SWMgr::isICU)
      swordxx::StringMgr::setSystemStringMgr(new BtStringMgr());
  \endcode
*/
class BtStringMgr : public swordxx::StringMgr {

    public:

        char *upperUTF8(char *text, unsigned int max = 0) const override;

        char *upperLatin1(char *text, unsigned int max = 0) const override;

    protected:

        bool supportsUnicode() const override;

};

#endif /* BTSTRINGMGR_H */
