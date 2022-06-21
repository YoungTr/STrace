package com.bomber.strace.core;

import android.os.Trace;
import android.util.Log;

import java.util.Stack;

public class SysTracer {
    private static final Stack<String> methodStack = new Stack<>();
    /**
     * 方法调用栈深度
     */
    private static final int maxSize = 6;

    public static void i(String className, String methodName) {
        String sectionName = className + "." + methodName;
        if (methodStack.size() <= maxSize) {
            methodStack.push(sectionName);
            Trace.beginSection(sectionName);
            Log.i("TAG", "SysTracer.i = " + sectionName);
        }
    }

    public static void o(String className, String methodName) {
        String sectionName = className + "." + methodName;

        if (methodStack.size() > 0) {
            methodStack.pop();
            Trace.endSection();
            Log.i("TAG", "SysTracer.o = " + sectionName);
        }
    }

    public static void catchIn(String className, String methodName) {
        String sectionName = className + "." + methodName;
        Trace.endSection();
        Log.i("TAG", "SysTracer.catchIn = " + sectionName);
    }
}