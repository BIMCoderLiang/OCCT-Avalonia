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

using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Platform;
using Avalonia.Threading;
using OCCT_NETWrapper;
using System;
using System.Runtime.InteropServices;

namespace OCCT_Avalonia
{
    public partial class OcctViewer : UserControl
    {
        private OcctNativeHost? _nativeHost;
        public OcctEngine? _engine;

        public OcctViewer()
        {
            _nativeHost = new OcctNativeHost(this);
            Content = _nativeHost;

            _nativeHost.HorizontalAlignment = HorizontalAlignment.Stretch;
            _nativeHost.VerticalAlignment = VerticalAlignment.Stretch;

            _nativeHost.MinWidth = 100;
            _nativeHost.MinHeight = 100;
        }

        protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
        {
            base.OnDetachedFromVisualTree(e);
            _engine?.Dispose();
            _engine = null;
        }

        private class OcctNativeHost : NativeControlHost
        {
            private readonly OcctViewer _parentViewer;
            private IntPtr _nativeWindowHandle = IntPtr.Zero;

            public OcctNativeHost(OcctViewer parent)
            {
                _parentViewer = parent;
                this.SizeChanged += OnSizeChanged;
            }

            protected override IPlatformHandle CreateNativeControlCore(IPlatformHandle parent)
            {
                if (_nativeWindowHandle != IntPtr.Zero)
                {
                    return new PlatformHandle(_nativeWindowHandle, "HWND");
                }

                try
                {
                    IntPtr parentHwnd = parent.Handle;
                    _nativeWindowHandle = CreateWin32ChildWindow(parentHwnd);

                    if (_nativeWindowHandle == IntPtr.Zero)
                    {
                        throw new Exception("Failed to create native child window for OCCT.");
                    }

                    Console.WriteLine($"[OCCT] Native Window Created: {_nativeWindowHandle}");

                    _parentViewer._engine = new OcctEngine(_nativeWindowHandle);
                    _parentViewer._engine.LoadNurbsSurface();

                    SetWindowText(_nativeWindowHandle, string.Empty);

                    var currentBounds = Bounds;
                    var topLevel = TopLevel.GetTopLevel(this);
                    var scaling = topLevel?.RenderScaling ?? 1.0;

                    int w = (int)(currentBounds.Width * scaling);
                    int h = (int)(currentBounds.Height * scaling);

                    Console.WriteLine($"[OCCT] Initial Size Push: {w} x {h}");

                    if (w > 0 && h > 0)
                    {
                        _parentViewer._engine.Resize(w, h);
                        _parentViewer._engine.Redraw();
                    }

                    UpdateWindow(_nativeWindowHandle);

                    Dispatcher.UIThread.Post(() =>
                    {
                        if (_nativeWindowHandle != IntPtr.Zero &&
                            _parentViewer._engine != null &&
                            !_parentViewer._engine.IsDisposed)
                        {
                            var bounds = _parentViewer.Bounds;
                            int finalW = (int)(bounds.Width * scaling);
                            int finalH = (int)(bounds.Height * scaling);

                            Console.WriteLine($"[OCCT] Dispatcher Resize: {finalW} x {finalH}");

                            if (finalW > 0 && finalH > 0)
                            {
                                _parentViewer._engine.Resize(finalW, finalH);
                            }
                            _parentViewer._engine.Redraw();
                            UpdateWindow(_nativeWindowHandle);
                        }
                    }, DispatcherPriority.Background);

                    return new PlatformHandle(_nativeWindowHandle, "HWND");
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[OCCT Error] {ex.Message}");
                    if (_nativeWindowHandle != IntPtr.Zero)
                    {
                        DestroyWindow(_nativeWindowHandle);
                        _nativeWindowHandle = IntPtr.Zero;
                    }
                    throw;
                }
            }

            protected override void DestroyNativeControlCore(IPlatformHandle handle)
            {
                if (_parentViewer._engine != null)
                {
                    _parentViewer._engine.Dispose();
                    _parentViewer._engine = null;
                }

                if (_nativeWindowHandle != IntPtr.Zero)
                {
                    DestroyWindow(_nativeWindowHandle);
                    _nativeWindowHandle = IntPtr.Zero;
                }
            }

            private void OnSizeChanged(object? sender, SizeChangedEventArgs e)
            {
                if (_parentViewer._engine == null || _parentViewer._engine.IsDisposed)
                    return;

                var topLevel = TopLevel.GetTopLevel(this);
                var scaling = topLevel?.RenderScaling ?? 1.0;

                int w = (int)(e.NewSize.Width * scaling);
                int h = (int)(e.NewSize.Height * scaling);

                Console.WriteLine($"[OCCT] SizeChanged Event: {w} x {h}");

                if (w > 0 && h > 0)
                {
                    _parentViewer._engine.Resize(w, h);
                    _parentViewer._engine.Redraw();
                }
            }

            #region Win32 API Helpers

            [DllImport("user32.dll", EntryPoint = "CreateWindowExW", SetLastError = true, CharSet = CharSet.Unicode)]
            private static extern IntPtr CreateWindowEx(
                int dwExStyle, string lpClassName, string lpWindowName,
                int dwStyle, int x, int y, int nWidth, int nHeight,
                IntPtr hWndParent, IntPtr hMenu, IntPtr hInstance, IntPtr lpParam);

            [DllImport("user32.dll", SetLastError = true)]
            [return: MarshalAs(UnmanagedType.Bool)]
            private static extern bool DestroyWindow(IntPtr hWnd);

            [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
            [return: MarshalAs(UnmanagedType.Bool)]
            private static extern bool SetWindowText(IntPtr hWnd, string lpString);

            [DllImport("user32.dll")]
            [return: MarshalAs(UnmanagedType.Bool)]
            private static extern bool UpdateWindow(IntPtr hWnd);

            private const int WS_CHILD = 0x40000000;
            private const int WS_VISIBLE = 0x10000000;
            private const int WS_CLIPCHILDREN = 0x02000000;
            private const int WS_CLIPSIBLINGS = 0x04000000;

            private IntPtr CreateWin32ChildWindow(IntPtr parentHwnd)
            {
                int style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

                IntPtr hWnd = CreateWindowEx(
                    0,
                    "STATIC",
                    "OCCT_Render_Target",
                    style,
                    0, 0, 100, 100,
                    parentHwnd,
                    IntPtr.Zero,
                    IntPtr.Zero,
                    IntPtr.Zero);

                if (hWnd == IntPtr.Zero)
                {
                    throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());
                }
                return hWnd;
            }

            #endregion
        }
    }
}