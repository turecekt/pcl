/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2020-, OpenPerception
 *
 *  \authors Antonio J Rodríguez-Sánchez, University of Innsbruck
 *           Tomáš Tureček, Tomas Bata University in Zlín
 *           Alex Melniciuc, University of Innsbruck
 *
 *  All rights reserved.
 */

#include <pcl/PCLPointCloud2.h>
#include <pcl/io/pcd_io.h>
#include <pcl/features/scurv.h>
#include <pcl/console/print.h>
#include <pcl/console/parse.h>
#include <pcl/console/time.h>

pcl::SCurVEstimation<pcl::PointNormal, pcl::PointNormal> scurv;

bool
loadCloud (const std::string &filename, pcl::PointCloud<pcl::PointNormal> &cloud)
{
  pcl::console::TicToc tt;
  pcl::console::print_highlight ("Loading "); pcl::console::print_value ("%s ", filename.c_str ());

  tt.tic ();
  if (pcl::io::loadPCDFile<pcl::PointNormal> (filename, cloud) < 0)
    return (false);
  pcl::console::print_info ("[done, ");
  pcl::console::print_value ("%g", tt.toc ());
  pcl::console::print_info (" ms : ");
  pcl::console::print_value ("%d", cloud.points.size()); pcl::console::print_info (" points]\n");
  pcl::console::print_info ("Available dimensions: "); pcl::console::print_value ("%s\n", getFieldsList (cloud).c_str ());

  // Check if the dataset has normals
  std::vector<pcl::PCLPointField> fields;
  if (getFieldIndex (cloud, "normal_x", fields) == -1)
  {
    pcl::console::print_error ("The input dataset does not contain normal information!\n");
    return (false);
  }
  return (true);
}

void
compute (const pcl::PointCloud<pcl::PointNormal>::Ptr &input, pcl::PointCloud<pcl::SCurVSignature210> &output)
{
  // Estimate
  pcl::console::TicToc tt;
  tt.tic ();
  
  pcl::console::print_highlight (stdout, "Computing with %d-nearest neighbors ", scurv.getKSearch());

  scurv.setSearchMethod (pcl::search::KdTree<pcl::PointNormal>::Ptr (new pcl::search::KdTree<pcl::PointNormal>));
  scurv.setInputCloud (input);
  scurv.setInputNormals (input);
 
  scurv.compute (output);

  pcl::console::print_info ("[done, ");
  pcl::console::print_value ("%g", tt.toc ());
  pcl::console::print_info (" ms : ");
  pcl::console::print_value ("%d", output.points.size());
  pcl::console::print_info (" points]\n");
}

void
saveCloud (const std::string &filename, const pcl::PointCloud<pcl::SCurVSignature210> &output)
{
  pcl::console::TicToc tt;
  tt.tic ();

  pcl::console::print_highlight ("Saving "); pcl::console::print_value ("%s ", filename.c_str ());
  
  pcl::io::savePCDFile (filename, output, false);
  
  pcl::console::print_info ("[done, ");
  pcl::console::print_value ("%g", tt.toc ());
  pcl::console::print_info (" ms : ");
  pcl::console::print_value ("%d", output.points.size());
  pcl::console::print_info (" points]\n");
}

/* ---[ */
int
main (int argc, char** argv)
{
  pcl::console::print_info ("Estimate SCurV (210) descriptors using pcl::SCurVEstimation. For more information, use: %s -h\n", argv[0]);
  bool help = false;
  pcl::console::parse_argument (argc, argv, "-h", help);
  if (argc < 3 || help)
  {
    pcl::console::print_error ("Syntax is: %s input.pcd output.pcd <options>\n", argv[0]);
    pcl::console::print_info ("  where options are:\n");
    pcl::console::print_info ("                     -k X      = use a fixed number of X-nearest neighbors around each point (default: "); 
    pcl::console::print_value ("%d", scurv.getKSearch()); 
    pcl::console::print_info (")\n"); 
    return 1;
  }

  // Parse the command line arguments for .pcd files
  const std::vector<int> p_file_indices = pcl::console::parse_file_extension_argument (argc, argv, ".pcd");
  if (p_file_indices.size () != 2)
  {
    pcl::console::print_error ("Need one input PCD file and one output PCD file to continue.\n");
    return 1;
  }

  int k;
  pcl::console::parse_argument (argc, argv, "-k", k);
  if ( k > 1 )
    scurv.setKSearch(k);

  // Load the first file
  pcl::PointCloud<pcl::PointNormal>::Ptr cloud (new pcl::PointCloud<pcl::PointNormal>);
  if (!loadCloud (argv[p_file_indices[0]], *cloud)) 
    return 1;

  // Perform the feature estimation
  pcl::PointCloud<pcl::SCurVSignature210> output;
  compute (cloud, output);

  // Save into the second file
  saveCloud (argv[p_file_indices[1]], output);
  return 0;
}
