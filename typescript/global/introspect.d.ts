type ScriptEvalResolvedCb = (value: object) => void;
type ScriptEvalRejectedCb = (except: object) => void;
type JournalLogLevel = 'debug' | 'info' | 'warning' | 'warn' |
                       'error' | 'err' | 'exception' | 'except';

type IntrospectSecurityPolicy = 'AllowLoadingSharedObject' | 'ForbidLoadingSharedObject' |
                                'AllowWritingToJournal' | 'ForbidWritingToJournal';

interface Introspect {
    setUncaughtExceptionHandler(handler: (except: object) => void): void;
    setBeforeExitHandler(handler: (exitCode: number) => void): void;
    loadSharedObject(path: string): void;
    scheduleScriptEvaluate(source: string, resolved?: ScriptEvalResolvedCb,
                           rejected?: ScriptEvalRejectedCb): void;
    scheduleModuleUrlEvaluate(url: string, resolved?: ScriptEvalResolvedCb,
                              rejected?: ScriptEvalRejectedCb): void;
    print(str: string): void;
    hasSyntheticModule(name: string): boolean;
    writeToJournal(level: JournalLogLevel, message: string): void;
    hasSecurityPolicy(policy: IntrospectSecurityPolicy): boolean;
}

declare let introspect: Introspect;
