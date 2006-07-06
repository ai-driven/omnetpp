package org.omnetpp.experimental.animation.modelchange;

import org.omnetpp.experimental.animation.model.RuntimeModule;
import org.omnetpp.experimental.animation.model.RuntimeSimulation;

public class ModuleCreation implements IRuntimeModelChange {
	private RuntimeSimulation simulation;
	private int id;
	private String moduleTypeName;
	private String parentFullPath;
	private String moduleName;
	private RuntimeModule module;
	
	public ModuleCreation(RuntimeSimulation simulation, 
			int id, String moduleTypeName, 
			String parentFullPath, String moduleName) {
		this.simulation = simulation;
		this.id = id;
		this.moduleTypeName = moduleTypeName;
		this.parentFullPath = parentFullPath;
		this.moduleName = moduleName;
	}
	
	public void execute() {
		RuntimeModule parent = simulation.getModuleByPath(parentFullPath);
		RuntimeModule mod = new RuntimeModule(parent);
		mod.setName(moduleName);
		mod.setTypeName(moduleTypeName);
		mod.setId(id);

		this.module = mod;
	}

	public void undo() {
		module.getParentModule().removeSubmodule(module);
		module = null;
		//TODO remove connections
	}

}
