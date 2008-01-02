package org.omnetpp.common.project;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.ArrayUtils;
import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectDescription;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.omnetpp.common.CommonPlugin;
import org.omnetpp.common.IConstants;
import org.omnetpp.common.util.FileUtils;
import org.omnetpp.common.util.StringUtils;


/**
 * Utilities to manage OMNeT++ projects.
 *
 * @author Andras
 */
public class ProjectUtils {
	public static final String NEDFOLDERS_FILENAME = ".nedfolders";

	/**
	 * Checks whether the the provided project is open, has the OMNeT++ nature,
	 * and it is enabled.
	 *
	 * Potential CoreExceptions are re-thrown as RuntimeException.
	 */
	public static boolean isOpenOmnetppProject(IProject project) {
		try {
			// project is open, nature is set and also enabled
			return project.isAccessible() && project.isNatureEnabled(IConstants.OMNETPP_NATURE_ID);
		}
		catch (CoreException e) {
			throw new RuntimeException(e);
		}
	}

	/**
	 * Returns all open projects with the OMNeT++ nature. Based on isOpenOmnetppProject().
	 */
	public static IProject[] getOmnetppProjects() {
		List<IProject> omnetppProjects = new ArrayList<IProject>();
        for (IProject project : ResourcesPlugin.getWorkspace().getRoot().getProjects())
        	if (isOpenOmnetppProject(project))
        		omnetppProjects.add(project);
        return omnetppProjects.toArray(new IProject[]{});
	}


	/**
	 * Returns the transitive closure of OMNeT++ projects referenced from the given project,
	 * excluding the project itself.
	 *
	 * Potential CoreExceptions are re-thrown as RuntimeException.
	 */
	public static IProject[] getAllReferencedOmnetppProjects(IProject project) {
		try {
			Set<IProject> result = new HashSet<IProject>();
			collectAllReferencedOmnetppProjects(project, true, result);
			return result.toArray(new IProject[]{});
		} catch (CoreException e) {
			throw new RuntimeException(e);
		}
	}

	/**
     * Returns the transitive closure of all projects referenced from the given project,
     * excluding the project itself.
     *
     * Potential CoreExceptions are re-thrown as RuntimeException.
     */
    public static IProject[] getAllReferencedProjects(IProject project) {
        try {
            Set<IProject> result = new HashSet<IProject>();
            collectAllReferencedOmnetppProjects(project, false, result);
            return result.toArray(new IProject[]{});
        } catch (CoreException e) {
            throw new RuntimeException(e);
        }
    }

	// helper for getAllReferencedOmnetppProjects()
	private static void collectAllReferencedOmnetppProjects(IProject project, boolean requireOmnetppNature, Set<IProject> result) throws CoreException {
		for (IProject dependency : project.getReferencedProjects()) {
			if ((requireOmnetppNature ? isOpenOmnetppProject(dependency) : project.isAccessible()) && !result.contains(dependency)) {
				result.add(dependency);
				collectAllReferencedOmnetppProjects(dependency, requireOmnetppNature, result);
			}
		}
	}

	public static boolean isNedFoldersFile(IResource resource) {
		return (resource instanceof IFile &&
				resource.getParent() instanceof IProject &&
				resource.getName().equals(NEDFOLDERS_FILENAME));
	}

	/**
	 * Reads the ".nedfolders" file from the given OMNeT++ project.
	 */
	public static IContainer[] readNedFoldersFile(IProject project) throws IOException, CoreException {
		List<IContainer> result = new ArrayList<IContainer>();
		IFile nedFoldersFile = project.getFile(NEDFOLDERS_FILENAME);
		if (nedFoldersFile.exists()) {
			String contents = FileUtils.readTextFile(nedFoldersFile.getContents());
			for (String line : StringUtils.splitToLines(contents)) {
				line = line.trim();
				if (line.equals("."))
					result.add(project);
				else if (line.length()>0)
					result.add(project.getFolder(line)); 
			}
		}
		if (result.isEmpty())
			result.add(project); // this is the default
		return result.toArray(new IContainer[]{});
	}

	/**
	 * Saves the ".nedfolders" file in the given OMNeT++ project.
	 */
    public static void saveNedFoldersFile(IProject project, IContainer[] folders) throws CoreException {
    	// assemble file content to save
    	String content = "";
    	for (IContainer element : folders)
    		content += getProjectRelativePathOf(project, element) + "\n";

    	// save it
    	IFile nedpathFile = project.getFile(NEDFOLDERS_FILENAME);
    	if (!nedpathFile.exists())
    		nedpathFile.create(new ByteArrayInputStream(content.getBytes()), IFile.FORCE, null);
    	else
    		nedpathFile.setContents(new ByteArrayInputStream(content.getBytes()), IFile.FORCE, null);
	}

	private static String getProjectRelativePathOf(IProject project, IContainer container) {
		return container.equals(project) ? "." : container.getProjectRelativePath().toString();
	}

    public static boolean hasOmnetppNature(IProject project) {
        try {
            IProjectDescription description = project.getDescription();
            String[] natures = description.getNatureIds();
            return ArrayUtils.contains(natures, IConstants.OMNETPP_NATURE_ID);
        } 
        catch (CoreException e) {
            CommonPlugin.logError(e);
            return false;
        }
    }

    /**
     * Add the omnetpp nature to the project (if the project does not have already)
     * @param project
     */
    public static void addOmnetppNature(IProject project) {
        try {
            if (hasOmnetppNature(project))
                return;
            IProjectDescription description = project.getDescription();
            String[] natures = description.getNatureIds();
            description.setNatureIds((String[])ArrayUtils.add(natures, IConstants.OMNETPP_NATURE_ID));
            project.setDescription(description, null);
            // note: builders are added automatically, by OmnetppNature.configure()
        } 
        catch (CoreException e) {
            CommonPlugin.logError(e);
        }
    }

    /**
     * Removes the omnetpp project nature
     */
    public static void removeOmnetppNature(IProject project) {
        try {
            if (!hasOmnetppNature(project))
                return;
            IProjectDescription description = project.getDescription();
            String[] natures = description.getNatureIds();
            description.setNatureIds((String[])ArrayUtils.removeElement(natures, IConstants.OMNETPP_NATURE_ID));
            project.setDescription(description, null);
            // note: builders are removed automatically, by OmnetppNature.deconfigure()
        } 
        catch (CoreException e) {
            CommonPlugin.logError(e);
        }
    }

}
