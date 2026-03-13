/*
 * OCCT-Avalonia
 *
 * Owner:
 * 2026/03/14 - Yuqing Liang (BIMCoder Liang)
 * bim.frankliang@foxmail.com
 *
 * COPYRIGHT:Yuqing Liang
 *
 * Use of this source code is governed by MIT LICENSE
 */

using System;
using System.Runtime.InteropServices;

namespace OCCT_NETWrapper
{
    public static class NativeMethods
    {
        private const string DllName = "OCCT_CAPI.dll";

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern IntPtr Occt_CreateEngine(IntPtr nativeWindowHandle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Occt_DestroyEngine(IntPtr engine);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Occt_LoadNurbsSurface(IntPtr engine);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Occt_View_FitAll(IntPtr engine);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Occt_View_Redraw(IntPtr engine);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Occt_View_Resize(IntPtr engine, int width, int height);
    }

    public class OcctEngine : IDisposable
    {
        private IntPtr _nativeEngine;
        private bool _disposed;

        public OcctEngine(IntPtr nativeWindowHandle)
        {
            if (nativeWindowHandle == IntPtr.Zero)
            {
                throw new ArgumentException("Native window handle cannot be zero.", nameof(nativeWindowHandle));
            }

            _nativeEngine = NativeMethods.Occt_CreateEngine(nativeWindowHandle);
            if (_nativeEngine == IntPtr.Zero)
            {
                throw new Exception("Failed to create OCCT engine. Check if the native DLL is present and compatible.");
            }
        }

        /// <summary>
        /// Gets a value indicating whether the engine has been disposed.
        /// </summary>
        public bool IsDisposed => _disposed;

        /// <summary>
        /// Loads a default NURBS surface into the current engine context.
        /// </summary>
        public void LoadNurbsSurface()
        {
            ThrowIfDisposed();
            NativeMethods.Occt_LoadNurbsSurface(_nativeEngine);
        }

        public void FitAll()
        {
            ThrowIfDisposed();
            NativeMethods.Occt_View_FitAll(_nativeEngine);
        }

        public void Redraw()
        {
            ThrowIfDisposed();
            NativeMethods.Occt_View_Redraw(_nativeEngine);
        }

        public void Resize(int w, int h)
        {
            ThrowIfDisposed();
            if (w <= 0 || h <= 0) return; 
            NativeMethods.Occt_View_Resize(_nativeEngine, w, h);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (_nativeEngine != IntPtr.Zero)
                {
                    NativeMethods.Occt_DestroyEngine(_nativeEngine);
                    _nativeEngine = IntPtr.Zero;
                }
                _disposed = true;
            }
        }

        ~OcctEngine()
        {
            Dispose(false);
        }

        private void ThrowIfDisposed()
        {
            if (_disposed)
            {
                throw new ObjectDisposedException(nameof(OcctEngine), "The OCCT engine has been disposed and cannot be used.");
            }
        }
    }
}