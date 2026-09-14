#ifndef DSTYPES_H
#define DSTYPES_H

#ifdef DYNLongDiagnostics
#define DYNDiagnosticBuf 10000
#define DYNDiagnosticStr "%.4000s"
#define DYNDiagnosticHalfStr "%.2000s"
#else /* DYNLongDiagnostics */
#define DYNDiagnosticBuf 1000
#define DYNDiagnosticStr "%.400s"
#define DYNDiagnosticHalfStr "%.200s"
#endif /* DYNLongDiagnostics */

#endif /* DSTYPES_H */
