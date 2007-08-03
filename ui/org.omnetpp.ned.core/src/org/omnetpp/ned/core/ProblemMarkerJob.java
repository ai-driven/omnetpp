package org.omnetpp.ned.core;


/**
 * This class updates the error markers in a single job (in background thread) its methods should
 * be synchronized, because it is possible that new files are added while the update process is running
 * @author rhornig
 */
//public class ProblemMarkerJob extends WorkspaceJob {
//
//    public static final String NEDPROBLEM_MARKERID = "org.omnetpp.ned.core.nedproblem";
//    public static final String NEDCONSISTENCYPROBLEM_MARKERID = "org.omnetpp.ned.core.nedconsistencyproblem";
//
//    protected HashMap<IFile, NEDErrorStore> nedParseErrors = new HashMap<IFile, NEDErrorStore>();
//    protected HashMap<IFile, NEDErrorStore> nedConsistencyErrors = new HashMap<IFile, NEDErrorStore>();
//
//    public ProblemMarkerJob(String name) {
//        super(name);
//        setPriority(INTERACTIVE);
//    }
//
//    /**
//     * Set the parsing error store for a file
//     */
//    public synchronized void setParseErrorStore(IFile file, NEDErrorStore errorStore) {
//        nedParseErrors.put(file, errorStore);
//    }
//
//    /**
//     * @param file
//     * @return The parse error store associated with the given file
//     */
//    public synchronized NEDErrorStore getParseErrorStore(IFile file) {
//        return nedParseErrors.get(file);
//    }
//
//    /**
//     * Set the consistency error store for a file
//     * @param file
//     * @param errorStore
//     */
//    public synchronized void setConsistencyErrorStore(IFile file, NEDErrorStore errorStore) {
//        nedConsistencyErrors.put(file, errorStore);
//    }
//
//    /**
//     * @param file
//     * @return The parse error store associated with the given file
//     */
//    public synchronized NEDErrorStore getConsistencyErrorStore(IFile file) {
//        return nedConsistencyErrors.get(file);
//    }
//    /**
//     * Removes all error stores associated with the file
//     * @param file
//     */
//    public synchronized void removeStores(IFile file) {
//        nedParseErrors.remove(file);
//        nedConsistencyErrors.remove(file);
//    }
//
//    /**
//     * @param file
//     * @return Whether the given file has errors in any of it's store
//     */
//    public boolean hasErrors(IFile file) {
//        NEDErrorStore es = nedParseErrors.get(file);
//        if (es != null && (es.containsError() || es.containsFatal()))
//            return true;
//
//        es = nedConsistencyErrors.get(file);
//        if (es != null && (es.containsError() || es.containsFatal()))
//            return true;
//
//        // otherwise we do not have any error
//        return false;
//    }
//
//    private void addMarkersToFile(IFile file, NEDErrorStore errors)
//            throws CoreException {
//        if (errors == null)
//            return;
//
//        // TODO we should first create all markers and later compare the created markers to the
//        // ones already on the file. we should add/delete markers only if there were changes in the
//        // error messages. this would avoid unnecessary marker refresh if we are editing a file which
//        // already contains markers. adding the markers should be done in a WorkspaceJob
//        // file.deleteMarkers(NEDPROBLEM_MARKERID, true, IResource.DEPTH_ZERO);
//        for (int i = 0; i < errors.numMessages(); i++) {
//            // XXX hack: parse out line number from string. NEDErrorStore should
//            // rather store line number as int...
//            String loc = errors.errorLocation(i);
//            int line = parseLineNumber(loc);
//            int markerSeverity = IMarker.SEVERITY_INFO;
//            NEDErrorCategory category = NEDErrorCategory.swigToEnum(errors.errorCategoryCode(i));
//            switch (category) {
//            case ERRCAT_FATAL:
//                markerSeverity = IMarker.SEVERITY_ERROR;
//                break;
//            case ERRCAT_ERROR:
//                markerSeverity = IMarker.SEVERITY_ERROR;
//                break;
//            case ERRCAT_WARNING:
//                markerSeverity = IMarker.SEVERITY_WARNING;
//                break;
//            case ERRCAT_INFO:
//                markerSeverity = IMarker.SEVERITY_INFO;
//                break;
//            default:
//                markerSeverity = IMarker.SEVERITY_ERROR;
//                break;
//            }
//            addMarker(file, NEDPROBLEM_MARKERID, markerSeverity, errors.errorText(i), line);
//        }
//    }
//
//    private int parseLineNumber(String loc) {
//        if (loc == null) return 1;
//        StringTokenizer t = new StringTokenizer(loc,":");
//        while (t.hasMoreTokens()) loc = t.nextToken();
//        int line = 1;
//        try {line = Integer.parseInt(loc);} catch (Exception e) {}
//        return line;
//    }
//
//    private void addMarker(final IFile file, final String type, int severity, String message, int line) throws CoreException {
//        IMarker marker= file.createMarker(type);
//        marker.setAttribute(IMarker.MESSAGE, message);
//        marker.setAttribute(IMarker.SEVERITY, severity);
//        marker.setAttribute(IMarker.LINE_NUMBER, line);
//    }
//
//
//    /**
//     * Runs the marker update job. Converts all messages in the ErrorStores to
//     * Markers and synchronizes them with fileMarkers already present on the file.
//     */
//    @Override
//    public synchronized IStatus runInWorkspace(IProgressMonitor monitor) throws CoreException {
//        // Converts all error store info to markers and attaches them to the files
//        for (IFile file : nedParseErrors.keySet()) {
//
//            if (!file.exists())
//                continue;
//            // TODO optimize the marker update. if no change occurred to the marker state
//            // do not touch the markers
//            // XXX see ProblemMarkerSynchronizer in Common!!! it solves exactly that problem  --Andras
//            file.deleteMarkers(NEDPROBLEM_MARKERID, true, IResource.DEPTH_ZERO);
//            file.deleteMarkers(NEDCONSISTENCYPROBLEM_MARKERID, true, IResource.DEPTH_ZERO);
//            addMarkersToFile(file, nedParseErrors.get(file));
//            addMarkersToFile(file, nedConsistencyErrors.get(file));
//        }
//        return Status.OK_STATUS;
//    }
//
//}
