-- phpMyAdmin SQL Dump
-- version 3.3.2
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: May 07, 2010 at 03:51 PM
-- Server version: 5.1.45
-- PHP Version: 5.3.2

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `smsg`
--

-- --------------------------------------------------------

--
-- Table structure for table `friend`
--

CREATE TABLE IF NOT EXISTS `friend` (
  `buddy1` int(11) NOT NULL,
  `buddy2` int(11) NOT NULL,
  PRIMARY KEY (`buddy1`,`buddy2`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `friend`
--

INSERT INTO `friend` (`buddy1`, `buddy2`) VALUES
(1, 4),
(1, 32),
(4, 1),
(4, 3),
(4, 5),
(4, 8),
(4, 11),
(4, 27),
(4, 29),
(32, 1);

-- --------------------------------------------------------

--
-- Table structure for table `presence`
--

CREATE TABLE IF NOT EXISTS `presence` (
  `userid` int(11) NOT NULL,
  `sd` int(11) NOT NULL,
  `ip` varchar(64) NOT NULL
) ENGINE=MEMORY DEFAULT CHARSET=latin1;

--
-- Dumping data for table `presence`
--


-- --------------------------------------------------------

--
-- Table structure for table `user`
--

CREATE TABLE IF NOT EXISTS `user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(256) NOT NULL,
  `password` varchar(256) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `username` (`username`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=33 ;

--
-- Dumping data for table `user`
--

INSERT INTO `user` (`id`, `username`, `password`) VALUES
(1, 'jiwandono', 'ponorogo'),
(3, 'ananti', 'ananto'),
(4, 'gin2_fm', 'gin'),
(5, 'coba', 'cobi'),
(8, 'bambang', 'bambang'),
(11, 'joko', 'joko'),
(25, 'jokoc', 'joko'),
(26, 'gentho', 'let'),
(27, 'bimo', 'bimo'),
(28, 'yudho', 'haha'),
(29, 'g_mie_ta', 'alay'),
(30, 'anantialay', 'betulsekali'),
(31, 'anantijelek', 'iya'),
(32, 'register', 'saya');
