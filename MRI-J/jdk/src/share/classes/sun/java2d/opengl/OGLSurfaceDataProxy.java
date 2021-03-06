/*
 * Copyright 2007 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Sun designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Sun in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 */

package sun.java2d.opengl;

import java.awt.Color;
import java.awt.Transparency;

import sun.java2d.SurfaceData;
import sun.java2d.SurfaceDataProxy;
import sun.java2d.loops.SurfaceType;
import sun.java2d.loops.CompositeType;

/**
 * The proxy class contains the logic for when to replace a
 * SurfaceData with a cached OGL Texture and the code to create
 * the accelerated surfaces.
 */
public class OGLSurfaceDataProxy extends SurfaceDataProxy {
    public static SurfaceDataProxy createProxy(SurfaceData srcData,
                                               OGLGraphicsConfig dstConfig)
    {
        if (srcData instanceof OGLSurfaceData) {
            // srcData must be a VolatileImage which either matches
            // our pixel format or not - either way we do not cache it...
            return UNCACHED;
        }

        return new OGLSurfaceDataProxy(dstConfig, srcData.getTransparency());
    }

    OGLGraphicsConfig oglgc;
    int transparency;

    public OGLSurfaceDataProxy(OGLGraphicsConfig oglgc, int transparency) {
        this.oglgc = oglgc;
        this.transparency = transparency;
    }

    @Override
    public SurfaceData validateSurfaceData(SurfaceData srcData,
                                           SurfaceData cachedData,
                                           int w, int h)
    {
        if (cachedData == null) {
            cachedData = oglgc.createManagedSurface(w, h, transparency);
        }
        return cachedData;
    }

    @Override
    public boolean isSupportedOperation(SurfaceData srcData,
                                        int txtype,
                                        CompositeType comp,
                                        Color bgColor)
    {
        return (bgColor == null || transparency == Transparency.OPAQUE);
    }
}
