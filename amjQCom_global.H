#ifndef AMJQCOM_GLOBAL_H
#define AMJQCOM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(AMJQCOM_LIBRARY)
#define AMJQCOM_EXPORT Q_DECL_EXPORT
#else
#define AMJQCOM_EXPORT Q_DECL_IMPORT
#endif

#endif// AMJQCOM_GLOBAL_H
