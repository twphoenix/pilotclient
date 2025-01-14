/* Copyright (C) 2016
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "blackmisc/db/registermetadatadb.h"
#include "blackmisc/variant.h"

// DB headers
#include "blackmisc/db/dbinfolist.h"
#include "blackmisc/db/dbinfo.h"
#include "blackmisc/db/dbflags.h"
#include "blackmisc/db/distribution.h"
#include "blackmisc/db/distributionlist.h"
#include "blackmisc/db/artifact.h"
#include "blackmisc/db/artifactlist.h"
#include "blackmisc/db/updateinfo.h"

namespace BlackMisc::Db
{
    void registerMetadata()
    {
        CDbFlags::registerMetadata();
        CDbInfo::registerMetadata();
        CDbInfoList::registerMetadata();
        CArtifact::registerMetadata();
        CArtifactList::registerMetadata();
        CDistribution::registerMetadata();
        CDistributionList::registerMetadata();
        CUpdateInfo::registerMetadata();
    }

} // ns
