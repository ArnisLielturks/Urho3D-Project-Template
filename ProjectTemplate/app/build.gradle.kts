plugins {
    id("com.android.application")
    kotlin("android")
    kotlin("android.extensions")
}

val kotlinVersion: String by ext
val ndkSideBySideVersion: String by ext
val cmakeVersion: String by ext
val buildStagingDir: String by ext

android {
    ndkVersion = ndkSideBySideVersion
    compileSdkVersion(30)
    defaultConfig {
        minSdkVersion(18)
        targetSdkVersion(30)
        applicationId = "io.urho3d.ProjectTemplate"
        versionCode = 1
        versionName = "1.0"
        testInstrumentationRunner = "android.support.test.runner.AndroidJUnitRunner"
        externalNativeBuild {
            cmake {
                arguments.apply {
                    System.getenv("ANDROID_CCACHE")?.let { add("-D ANDROID_CCACHE=$it") }
                    add("-D JNI_DIR=${project.file(buildStagingDir)}")
                    // Pass along matching env-vars as CMake build options
                    addAll(project.file("../script/.build-options")
                        .readLines()
                        .mapNotNull { variable -> System.getenv(variable)?.let { "-D $variable=$it" } }
                    )
                }
            }
        }
        splits {
            abi {
                isEnable = project.hasProperty("ANDROID_ABI")
                reset()
                include(
                    *(project.findProperty("ANDROID_ABI") as String? ?: "")
                        .split(',')
                        .toTypedArray()
                )
            }
        }
    }
    buildTypes {
        named("release") {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }
    lintOptions {
        isAbortOnError = false
    }
    externalNativeBuild {
        cmake {
            version = cmakeVersion
            path = project.file("../CMakeLists.txt")
            setBuildStagingDirectory(buildStagingDir)
        }
    }
    sourceSets {
        named("main") {
            assets.srcDir(project.file("../bin"))
        }
    }
}

val urhoReleaseImpl by configurations.creating { isCanBeResolved = true }
configurations.releaseImplementation.get().extendsFrom(urhoReleaseImpl)

val urhoDebugImpl by configurations.creating { isCanBeResolved = true }
configurations.debugImplementation.get().extendsFrom(urhoDebugImpl)

dependencies {
    urhoReleaseImpl("io.urho3d:urho3d-lib-static:Unversioned")
    urhoDebugImpl("io.urho3d:urho3d-lib-static-debug:Unversioned")
    implementation(fileTree(mapOf("dir" to "libs", "include" to listOf("*.jar", "*.aar"))))
    implementation("org.jetbrains.kotlin:kotlin-stdlib-jdk8:$kotlinVersion")
    implementation("androidx.core:core-ktx:1.3.2")
    implementation("androidx.appcompat:appcompat:1.2.0")
    implementation("androidx.constraintlayout:constraintlayout:2.0.2")
    testImplementation("junit:junit:4.13.1")
    androidTestImplementation("androidx.test:runner:1.3.0")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.3.0")
}

afterEvaluate {
    android.buildTypes.forEach { buildType ->
        val config = buildType.name.capitalize()
        val unzipTaskName = "unzipJni$config"
        tasks {
            "generateJsonModel$config" {
                dependsOn(unzipTaskName)
            }
            register<Copy>(unzipTaskName) {
                val aar = configurations["urho${config}Impl"].resolve().first { it.name.startsWith("urho3d-lib") }
                from(zipTree(aar))
                include("urho3d/**")
                into(android.externalNativeBuild.cmake.buildStagingDirectory)
            }
        }
    }
}

tasks {
    register<Delete>("cleanAll") {
        dependsOn("clean")
        delete = setOf(android.externalNativeBuild.cmake.buildStagingDirectory)
    }
}
